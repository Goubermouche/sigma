#include "ir_translator.h"

#include <compiler/compiler/compilation_context.h>
#include <utility/string_helper.h>

namespace sigma {
	auto ir_translator::translate(backend_context& context) -> utility::result<void> {
		return ir_translator(context).translate();
	}

	ir_translator::ir_translator(backend_context& context) : m_context(context) {
		m_context.semantics.reset_active_scope();
	}

	auto ir_translator::translate_node(handle<ast::node> ast_node) -> handle<ir::node> {
		switch(ast_node->type) {
			// declarations
			case ast::node_type::NAMESPACE_DECLARATION:          translate_namespace_declaration(ast_node); break;
			case ast::node_type::FUNCTION_DECLARATION:           translate_function_declaration(ast_node); break;
			case ast::node_type::VARIABLE_DECLARATION:           translate_variable_declaration(ast_node); break;

			// literals
			case ast::node_type::NUMERICAL_LITERAL:              return translate_numerical_literal(ast_node);
			case ast::node_type::CHARACTER_LITERAL:              return translate_character_literal(ast_node);
			case ast::node_type::STRING_LITERAL:                 return translate_string_literal(ast_node);
			case ast::node_type::BOOL_LITERAL:                   return translate_bool_literal(ast_node);

			// expressions
			case ast::node_type::OPERATOR_ADD:
			case ast::node_type::OPERATOR_SUBTRACT:
			case ast::node_type::OPERATOR_MULTIPLY:
			case ast::node_type::OPERATOR_DIVIDE:
			case ast::node_type::OPERATOR_MODULO:                return translate_binary_math_operator(ast_node);
			case ast::node_type::OPERATOR_GREATER_THAN_OR_EQUAL:
			case ast::node_type::OPERATOR_LESS_THAN_OR_EQUAL:
			case ast::node_type::OPERATOR_GREATER_THAN:
			case ast::node_type::OPERATOR_LESS_THAN:             return translate_binary_comparison_operator(ast_node);
			case ast::node_type::OPERATOR_NOT_EQUAL:
			case ast::node_type::OPERATOR_EQUAL:                 return translate_binary_equality_operator(ast_node);
				
			case ast::node_type::OPERATOR_CONJUNCTION:
			case ast::node_type::OPERATOR_DISJUNCTION:           return translate_predicate_operator(ast_node);

			// statements
			case ast::node_type::RETURN:                         translate_return(ast_node); break;
			case ast::node_type::CONDITIONAL_BRANCH:             translate_conditional_branch(ast_node, nullptr); break;

			// loads / stores
			case ast::node_type::VARIABLE_ACCESS:                return translate_variable_access(ast_node);
			case ast::node_type::ARRAY_ACCESS:                   return translate_array_access(ast_node);
			case ast::node_type::STORE:                          return translate_store(ast_node);
			case ast::node_type::LOAD:                           return translate_load(ast_node);

			// other
			case ast::node_type::FUNCTION_CALL:                  return translate_function_call(ast_node);
			case ast::node_type::SIZEOF:                         return translate_sizeof(ast_node);
			case ast::node_type::CAST:                           return translate_cast(ast_node);

			default: PANIC("irgen for node '{}' is not implemented", ast_node->type.to_string());
		}

		return nullptr;
	}

	void ir_translator::translate_function_declaration(handle<ast::node> function_node) {
		const ast::function& function = function_node->get<ast::function>();

		m_context.semantics.declare_local_function(function.signature);
		m_context.semantics.trace_push_scope();

		// TODO: handle varargs
		// declare parameter temporaries
		for (u64 i = 0; i < function.signature.parameter_types.get_size(); ++i) {
			const auto variable = m_context.semantics.get_variable(function.signature.parameter_types[i].identifier_key);
			ASSERT(variable, "function parameter pre declaration is invalid");

			// since we can't update the projection value directly we have to create a proxy for it, this
			// allows us to update the value of our parameters
			const u16 byte_width = function.signature.parameter_types[i].type.get_byte_width();
			variable->value = m_context.builder.create_local(byte_width, byte_width);

			// assign the parameter value to the proxy
			const handle<ir::node> parameter_projection = m_context.builder.get_function_parameter(i);
			m_context.builder.create_store(variable->value, parameter_projection, byte_width, false);
		}

		// handle inner statements
		for (const handle<ast::node>& statement : function_node->children) {
			translate_node(statement);
		}

		m_context.semantics.declare_implicit_return();
		m_context.semantics.trace_pop_scope();
	}

	void ir_translator::translate_variable_declaration(handle<ast::node> variable_node) {
		const auto& prop = variable_node->get<ast::named_type_expression>();
		const u16 byte_width = prop.type.get_byte_width();
		const handle<ir::node> local = m_context.semantics.declare_variable(prop.key, byte_width, byte_width);

		if (variable_node->children.get_size() == 1) {
			const handle<ir::node> expression = translate_node(variable_node->children[0]);
			m_context.builder.create_store(local, expression, byte_width, false);
		}
		else if (prop.type.base_type == data_type::BOOL) {
			// boolean values should default to false
			m_context.builder.create_store(local, m_context.builder.create_bool(false), byte_width, false);
		}
	}

	void ir_translator::translate_namespace_declaration(handle<ast::node> namespace_node) {
		m_context.semantics.trace_push_scope();

		for(const handle<ast::node> statement : namespace_node->children) {
			translate_node(statement);
		}

		m_context.semantics.trace_pop_scope();
	}

	void ir_translator::translate_return(handle<ast::node> return_node) {
		if (return_node->children.get_size() == 0) {
			m_context.builder.create_return({}); // TODO: maybe this should return a VOID_TY?
		}
		else {
			m_context.builder.create_return({ translate_node(return_node->children[0]) });
		}
	}

	auto ir_translator::translate_sizeof(handle<ast::node> sizeof_node) const -> handle<ir::node> {
		const ast::type_expression& sizeof_value = sizeof_node->get<ast::type_expression>();
		const u16 byte_width = sizeof_value.type.get_byte_width();

		return m_context.builder.create_unsigned_integer(byte_width, 64);
	}

	void ir_translator::translate_conditional_branch(handle<ast::node> branch_node, handle<ir::node> end_control) {
		const handle<ir::node> true_control = m_context.builder.create_region();
		const handle<ir::node> false_control = m_context.builder.create_region();
		end_control = end_control ? end_control : m_context.builder.create_region();

		// translate the condition
		const handle<ir::node> condition = translate_node(branch_node->children[0]);

		// check if there is a successor branch node
		if (const handle<ast::node> successor = branch_node->children[1]) {
			// successor node
			m_context.builder.create_conditional_branch(condition, true_control, false_control);
			m_context.builder.set_control(false_control);

			[[likely]]
			if (successor->type == ast::node_type::CONDITIONAL_BRANCH) {
				// else if
				translate_conditional_branch(successor, end_control);
			}
			else if (successor->type == ast::node_type::BRANCH) {
				// else
				translate_branch(successor, end_control);
			}
			else {
				PANIC("unreachable");
			}
		}
		else {
			m_context.builder.create_conditional_branch(condition, true_control, end_control);
		}

		// this all happens if CONDITION IS true
		m_context.builder.set_control(true_control);
		m_context.semantics.trace_push_scope();

		for (u64 i = 2; i < branch_node->children.get_size(); ++i) {
			translate_node(branch_node->children[i]);
		}

		m_context.semantics.trace_pop_scope();
		m_context.builder.create_branch(end_control);

		// restore the control region
		m_context.builder.set_control(end_control);
	}

	void ir_translator::translate_branch(handle<ast::node> branch_node, handle<ir::node> exit_control) {
		m_context.semantics.trace_push_scope();

		for (const handle<ast::node>& statement : branch_node->children) {
			translate_node(statement);
		}

		m_context.semantics.trace_pop_scope();
		m_context.builder.create_branch(exit_control);
	}

	auto ir_translator::translate_array_access(handle<ast::node> access_node) -> handle<ir::node> {
		// number of array accesses we need to perform
		const u16 access_level = access_node->children.get_size() - 1;

		// store the base type for the current operation
		data_type base_type = access_node->get<ast::type_expression>().type;
		handle<ir::node> base = translate_node(access_node->children[0]);

		// generate individual accesses
		for(u64 i = 0; i < access_level; ++i) {
			const handle<ir::node> index = translate_node(access_node->children[i + 1]);
			const data_type accessed_type = base_type.create_access(1);
			const u16 type_width = accessed_type.get_byte_width();

			base = m_context.builder.create_array_access(base, index, type_width);

			if(i + 1 < access_level) {
				// if the access has more than one level we have to load each pointer and
				// then access it further
				const ir::data_type type = detail::data_type_to_ir(accessed_type);
				base = m_context.builder.create_load(base, type, type_width, false);

				base_type = base_type.create_access(1);
			}
		}

		// return the last access
		return base;
	}

	auto ir_translator::translate_numerical_literal(handle<ast::node> numerical_literal_node) const -> handle<ir::node> {
		return literal_to_ir(numerical_literal_node->get<ast::named_type_expression>());
	}

	auto ir_translator::translate_character_literal(handle<ast::node> character_literal_node) const -> handle<ir::node> {
		const std::string& value = m_context.syntax.strings.get(character_literal_node->get<ast::named_type_expression>().key);
		ASSERT(value.size() == 1, "invalid char literal length");
		return m_context.builder.create_signed_integer(value[0], 32);
	}

	auto ir_translator::translate_string_literal(handle<ast::node> string_literal_node) const -> handle<ir::node> {
		const std::string& value = m_context.syntax.strings.get(string_literal_node->get<ast::named_type_expression>().key);
		return m_context.builder.create_string(value);
	}

	auto ir_translator::translate_bool_literal(handle<ast::node> bool_literal_node) const -> handle<ir::node> {
		const auto& prop = bool_literal_node->get<ast::bool_literal>();
		return m_context.builder.create_bool(prop.value);
	}

	auto ir_translator::translate_binary_math_operator(handle<ast::node> operator_node) -> handle<ir::node> {
		const handle<ir::node> left  = translate_node(operator_node->children[0]);
		const handle<ir::node> right = translate_node(operator_node->children[1]);

		switch(operator_node->type) {
			case ast::node_type::OPERATOR_ADD:      return m_context.builder.create_add(left, right);
			case ast::node_type::OPERATOR_SUBTRACT: return m_context.builder.create_sub(left, right);
			case ast::node_type::OPERATOR_MULTIPLY: return m_context.builder.create_mul(left, right);
			//case node_type::OPERATOR_DIVIDE:   
			//case node_type::OPERATOR_MODULO:
			default: PANIC("unexpected node type '{}' received", operator_node->type.to_string());
		}

		return nullptr; // unreachable
	}

	auto ir_translator::translate_binary_comparison_operator(handle<ast::node> operator_node) -> handle<ir::node> {
		const handle<ir::node> left = translate_node(operator_node->children[0]);
		const handle<ir::node> right = translate_node(operator_node->children[1]);

		// determine the comparison operator type
		const ast::comparison_expression& expression = operator_node->get<ast::comparison_expression>();

		if(expression.type == ast::comparison_expression::type::POINTER) {
			// pointer comparisons
			switch (operator_node->type) {
				case ast::node_type::OPERATOR_GREATER_THAN_OR_EQUAL: return m_context.builder.create_cmp_ige(left, right, false);
				case ast::node_type::OPERATOR_LESS_THAN_OR_EQUAL:    return m_context.builder.create_cmp_ile(left, right, false);
				case ast::node_type::OPERATOR_GREATER_THAN:          return m_context.builder.create_cmp_igt(left, right, false);
				case ast::node_type::OPERATOR_LESS_THAN:             return m_context.builder.create_cmp_ilt(left, right, false);
				default: PANIC("unexpected node type '{}' received", operator_node->type.to_string());
			}
		}
		else if(expression.type == ast::comparison_expression::type::FLOATING_POINT) {
			// floating point comparisons 
			NOT_IMPLEMENTED();
		}
		else {
			// integral comparisons
			// INTEGRAL_SIGNED || INTEGRAL_UNSIGNED
			const bool is_signed = expression.type == ast::comparison_expression::type::INTEGRAL_SIGNED;

			switch (operator_node->type) {
				case ast::node_type::OPERATOR_GREATER_THAN_OR_EQUAL: return m_context.builder.create_cmp_ige(left, right, is_signed);
				case ast::node_type::OPERATOR_LESS_THAN_OR_EQUAL:    return m_context.builder.create_cmp_ile(left, right, is_signed);
				case ast::node_type::OPERATOR_GREATER_THAN:          return m_context.builder.create_cmp_igt(left, right, is_signed);
				case ast::node_type::OPERATOR_LESS_THAN:             return m_context.builder.create_cmp_ilt(left, right, is_signed);
				default: PANIC("unexpected node type '{}' received", operator_node->type.to_string());
			}
		}

		return nullptr; // unreachable
	}

	auto ir_translator::translate_binary_equality_operator(handle<ast::node> operator_node) -> handle<ir::node> {
		const handle<ir::node> left = translate_node(operator_node->children[0]);
		const handle<ir::node> right = translate_node(operator_node->children[1]);

		switch (operator_node->type) {
			case ast::node_type::OPERATOR_NOT_EQUAL:             return m_context.builder.create_cmp_ne(left, right);
			case ast::node_type::OPERATOR_EQUAL:                 return m_context.builder.create_cmp_eq(left, right);
			default: PANIC("unexpected node type '{}' received", operator_node->type.to_string());
		}

		return nullptr; // unreachable
	}

	auto ir_translator::translate_predicate_operator(handle<ast::node> operator_node) -> handle<ir::node> {
		const handle<ir::node> left = translate_node(operator_node->children[0]);
		const handle<ir::node> right = translate_node(operator_node->children[1]);

		switch(operator_node->type) {
			case ast::node_type::OPERATOR_CONJUNCTION: return m_context.builder.create_and(left, right);
			case ast::node_type::OPERATOR_DISJUNCTION: return m_context.builder.create_or(left, right);
			default: PANIC("unexpected node type '{}' received", operator_node->type.to_string());
		}

		return nullptr; // unreachable
	}

	auto ir_translator::translate_cast(handle<ast::node> cast_node) -> handle<ir::node> {
		const ast::cast& cast = cast_node->get<ast::cast>();

		const handle<ir::node> value_to_cast = translate_node(cast_node->children[0]);
		const ir::data_type target_type = detail::data_type_to_ir(cast.target_type);

		if(cast.target_type.is_pointer()) {
			return value_to_cast; // just return the pointer
		}

		if(detail::determine_cast_kind(cast.original_type, cast.target_type)) {
			// truncate the original value
			return m_context.builder.create_truncate(value_to_cast, target_type);
		}

		if (cast.original_type.is_signed()) {
			// sign-extend the original value
			return m_context.builder.create_sxt(value_to_cast, target_type);
		}

		// zero-extend the original value
		return m_context.builder.create_zxt(value_to_cast, target_type);
	}

	auto ir_translator::translate_function_call(handle<ast::node> call_node) -> handle<ir::node> {
		const ast::function_call& callee = call_node->get<ast::function_call>();

		std::vector<handle<ir::node>> parameters;
		parameters.reserve(call_node->children.get_size());

		for (const handle<ast::node>& parameter : call_node->children) {
			parameters.push_back(translate_node(parameter));
		}

		const handle<ir::node> call_result = m_context.semantics.create_call(callee.signature, callee.namespaces, parameters);
		return call_result;
	}

	auto ir_translator::translate_load(handle<ast::node> load_node) -> handle<ir::node> {
		// translate the target value
		const handle<ir::node> value_to_load = translate_node(load_node->children[0]);

		// get the type and alignment of the load
		const data_type& type_to_load = load_node->get<ast::type_expression>().type;
		const ir::data_type ir_type = detail::data_type_to_ir(type_to_load);
		const u16 alignment = type_to_load.get_byte_width();

		// create the load operation
		return m_context.builder.create_load(value_to_load, ir_type, alignment, false);
	}

	auto ir_translator::translate_variable_access(handle<ast::node> access_node) const -> handle<ir::node> {
		const auto& accessed_variable = access_node->get<ast::named_type_expression>();

		// just return the variable value, at this point everything was handled by the type checker
		return m_context.semantics.get_variable(accessed_variable.key)->value;
	}

	auto ir_translator::translate_store(handle<ast::node> assignment_node) -> handle<ir::node> {
		const auto& variable = assignment_node->children[0]->get<ast::named_type_expression>();
		const u16 alignment = variable.type.get_byte_width();

		// get the value we want to store
		const handle<ir::node> value_to_store = translate_node(assignment_node->children[1]);

		// get the storage location
		const handle<ast::node> storage_node = assignment_node->children[0];
		const handle<ir::node> storage = translate_node(storage_node);

		m_context.builder.create_store(storage, value_to_store, alignment, false);
		return nullptr;
	}

	auto ir_translator::literal_to_ir(const ast::named_type_expression& literal) const -> handle<ir::node> {
		const std::string& value = m_context.syntax.strings.get(literal.key);

		// handle pointers separately
		if (literal.type.pointer_level > 0) {
			NOT_IMPLEMENTED();
		}

		bool overflow; // ignored

		switch (literal.type.base_type) {
			case data_type::I8:   return m_context.builder.create_signed_integer(utility::detail::from_string<i8>(value, overflow), 8);
			case data_type::I16:  return m_context.builder.create_signed_integer(utility::detail::from_string<i16>(value, overflow), 16);
			case data_type::I32:  return m_context.builder.create_signed_integer(utility::detail::from_string<i32>(value, overflow), 32);
			case data_type::I64:  return m_context.builder.create_signed_integer(utility::detail::from_string<i64>(value, overflow), 64);
			case data_type::U8:   return m_context.builder.create_unsigned_integer(utility::detail::from_string<u8>(value, overflow), 8);
			case data_type::U16:  return m_context.builder.create_unsigned_integer(utility::detail::from_string<u16>(value, overflow), 16);
			case data_type::U32:  return m_context.builder.create_unsigned_integer(utility::detail::from_string<u32>(value, overflow), 32);
			case data_type::U64:  return m_context.builder.create_unsigned_integer(utility::detail::from_string<u64>(value, overflow), 64);
			// for cases when a numerical literal is implicitly converted to a bool (ie. "if(1)")
			case data_type::BOOL: return m_context.builder.create_bool(utility::detail::from_string<bool>(value, overflow));
			default: NOT_IMPLEMENTED();
		}

		return nullptr;
	}

	auto ir_translator::translate() -> utility::result<void> {
		for (const handle<ast::node>& top_level : m_context.syntax.ast.get_nodes()) {
			translate_node(top_level);
		}

		return SUCCESS;
	}
} // namespace sigma
