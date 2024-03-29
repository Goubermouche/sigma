#include "ir_translator.h"
#include <compiler/compiler/compilation_context.h>

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
			case ast::node_type::VARIABLE_DECLARATION:           return translate_variable_declaration(ast_node);
			case ast::node_type::STRUCT_DECLARATION:             return nullptr; // we don't have to do anything here

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
			case ast::node_type::OPERATOR_LOGICAL_NOT:           return translate_logical_not_operator(ast_node);

			// statements
			case ast::node_type::RETURN:                         translate_return(ast_node); break;
			case ast::node_type::CONDITIONAL_BRANCH:             translate_conditional_branch(ast_node, nullptr); break;

			// loads / stores
			case ast::node_type::VARIABLE_ACCESS:                return translate_variable_access(ast_node);
			case ast::node_type::ARRAY_ACCESS:                   return translate_array_access(ast_node);
			case ast::node_type::LOCAL_MEMBER_ACCESS:            return translate_local_member_access(ast_node);
			case ast::node_type::STORE:                          return translate_store(ast_node);
			case ast::node_type::LOAD:                           return translate_load(ast_node);

			// other
			case ast::node_type::FUNCTION_CALL:                  return translate_function_call(ast_node);
			case ast::node_type::ALIGNOF:                        return translate_alignof(ast_node);
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
			const auto& parameter_type = function.signature.parameter_types[i].type;

			const u16 alignment = parameter_type.get_alignment();
			const u16 size = parameter_type.get_size();

			variable->value = m_context.builder.create_local(size, alignment);
			const handle<ir::node> projection = m_context.builder.get_function_parameter(i);

			if(parameter_type.is_struct()) {
				// copy over entire structs
				copy_struct(variable->value, projection, parameter_type);
			}
			else {
				// copy smaller values over
				// assign the parameter value to the proxy
				m_context.builder.create_store(variable->value, projection, alignment, false);
			}
		}

		// handle inner statements
		for (const handle<ast::node>& statement : function_node->children) {
			translate_node(statement);
		}

		m_context.semantics.define_implicit_return();
		m_context.semantics.trace_pop_scope();
	}

	auto ir_translator::translate_variable_declaration(handle<ast::node> variable_node) const -> handle<ir::node>{
		const auto& declaration = variable_node->get<ast::named_type_expression>();

		const u16 alignment = declaration.type.get_alignment();
		const u16 size = declaration.type.get_size();

		return m_context.semantics.declare_variable(declaration.key, size, alignment);
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

	auto ir_translator::translate_alignof(handle<ast::node> alignof_node) const -> handle<ir::node> {
		const ast::type_expression& alignof_value = alignof_node->get<ast::type_expression>();
		const u16 alignment = alignof_value.type.get_alignment();

		return m_context.builder.create_unsigned_integer(alignment, 64);
	}

	auto ir_translator::translate_sizeof(handle<ast::node> sizeof_node) const -> handle<ir::node> {
		const ast::type_expression& sizeof_value = sizeof_node->get<ast::type_expression>();
		const u16 size = sizeof_value.type.get_size();

		return m_context.builder.create_unsigned_integer(size, 64);
	}

	void ir_translator::translate_conditional_branch(handle<ast::node> branch_node, handle<ir::node> end_control) {
		const handle<ir::node> true_control = m_context.builder.create_region();
		end_control = end_control ? end_control : m_context.builder.create_region();

		// translate the condition
		const handle<ir::node> condition = translate_node(branch_node->children[0]);

		// check if there is a successor branch node
		if (const handle<ast::node> successor = branch_node->children[1]) {
			const handle<ir::node> false_control = m_context.builder.create_region();

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

		for(u64 i = 2; i < branch_node->children.get_size(); ++i) {
			translate_node(branch_node->children[i]);
		}

		if(!m_context.semantics.has_return()) {
			// if we don't have a return statement in this scope, we have to branch back
			m_context.builder.create_branch(end_control);
		}

		m_context.semantics.trace_pop_scope();

		// restore the control region
		m_context.builder.set_control(end_control);
	}

	void ir_translator::translate_branch(handle<ast::node> branch_node, handle<ir::node> exit_control) {
		m_context.semantics.trace_push_scope();

		for (const handle<ast::node>& statement : branch_node->children) {
			translate_node(statement);
		}

		if (!m_context.semantics.has_return()) {
			// if we don't have a return statement in this scope, we have to branch back
			m_context.builder.create_branch(exit_control);
		}

		m_context.semantics.trace_pop_scope();
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

		u64 value = 0;

		if(prop.value) {
			value = 1;
		}

		return m_context.builder.create_unsigned_integer(value, 8);
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

		if(expression.type == ast::comparison_expression::type::FLOATING_POINT) {
			// floating point comparisons 
			NOT_IMPLEMENTED();
		}
		else {
			// integral comparisons
			bool is_signed;

			if(expression.type == ast::comparison_expression::type::POINTER) {
				// interpret pointers as unsigned
				is_signed = false;
			}
			else {
				is_signed = expression.type == ast::comparison_expression::type::INTEGRAL_SIGNED;
			}

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

	auto ir_translator::translate_logical_not_operator(handle<ast::node> operator_node) -> handle<ir::node> {
		// evaluate the boolean expression we want to negate
		const handle<ir::node> expression = translate_node(operator_node->children[0]);

		// negate it
		// NOTE: booleans are 8 bits
		return m_context.builder.create_cmp_eq(expression, m_context.builder.create_unsigned_integer(0, 8));
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

		const handle<ir::node> call_result = m_context.semantics.create_call(
			callee.signature, callee.namespaces, parameters
		);

		return call_result;
	}

	auto ir_translator::translate_load(handle<ast::node> load_node) -> handle<ir::node> {
		// translate the target value
		const handle<ir::node> value_to_load = translate_node(load_node->children[0]);

		// get the type and alignment of the load
		const type& type_to_load = load_node->get<ast::type_expression>().type;
		const ir::data_type ir_type = detail::data_type_to_ir(type_to_load);
		const u16 alignment = type_to_load.get_alignment();

		if(type_to_load.is_struct()) {
			return value_to_load;
		}

		// create the load operation
		return m_context.builder.create_load(value_to_load, ir_type, alignment, false);
	}

	auto ir_translator::translate_array_access(handle<ast::node> access_node) -> handle<ir::node> {
		// translate the storage location
		handle<ir::node> base = translate_node(access_node->children[0]);

		const type base_type = access_node->get<ast::type_expression>().type;
		const u16 alignment = base_type.dereference(1).get_alignment(); // get the alignment of the type we're accessing

		// chained accesses
		if (
			base->get_type() == ir::node::type::ARRAY_ACCESS ||
			base->get_type() == ir::node::type::MEMBER_ACCESS
		) {
			const ir::data_type type = detail::data_type_to_ir(base_type);
			base = m_context.builder.create_load(base, type, alignment, false);
		}
		else if (base->get_type() == ir::node::type::PROJECTION) { /* does nothing */ }
		else {
			ASSERT(base->get_type() == ir::node::type::LOCAL, "unhandled node type");
		}

		// translate the index
		const handle<ir::node> index = translate_node(access_node->children[1]);

		return m_context.builder.create_array_access(base, index, alignment);
	}

	auto ir_translator::translate_local_member_access(handle<ast::node> access_node) -> handle<ir::node> {
	  const auto& access = access_node->get<ast::named_type_expression>();

		const handle<ir::node> base = translate_node(access_node->children[0]);
		// ASSERT(
		// 	base->get_type() == ir::node::type::LOCAL || base->get_type() == ir::node::type::PROJECTION, 
		// 	"unhandled node type"
		// );

		const type& base_type = access_node->children[0]->get<ast::named_type_expression>().type;
		const u16 offset = base_type.get_member_offset(access.key);

		return m_context.builder.create_member_access(base, offset);
	}

	auto ir_translator::translate_variable_access(handle<ast::node> access_node) const -> handle<ir::node> {
		const auto& accessed_variable = access_node->get<ast::named_type_expression>();

		// just return the variable value, at this point everything was handled by the type checker
		return m_context.semantics.get_variable(accessed_variable.key)->value;
	}

	auto ir_translator::translate_store(handle<ast::node> assignment_node) -> handle<ir::node> {
		const auto& variable = assignment_node->children[0]->get<ast::named_type_expression>();
		const u16 alignment = variable.type.get_alignment();

		// get the value we want to store
		const handle<ir::node> value_to_store = translate_node(assignment_node->children[1]);

		// get the storage location
		const handle<ast::node> storage_node = assignment_node->children[0];
		const handle<ir::node> storage = translate_node(storage_node);

		if(variable.type.is_struct()) {
			copy_struct(storage, value_to_store, variable.type);
		}
		else {
			m_context.builder.create_store(storage, value_to_store, alignment, false);
		}

		return nullptr;
	}

	auto ir_translator::literal_to_ir(const ast::named_type_expression& literal) const -> handle<ir::node> {
		const std::string& value = m_context.syntax.strings.get(literal.key);

		// handle pointers separately
		if (literal.type.is_pointer()) {
			NOT_IMPLEMENTED();
		}

		bool overflow; // ignored

		switch (literal.type.get_kind()) {
			case type::I8:   return m_context.builder.create_signed_integer(utility::from_string<i8>(value, overflow), 8);
			case type::I16:  return m_context.builder.create_signed_integer(utility::from_string<i16>(value, overflow), 16);
			case type::I32:  return m_context.builder.create_signed_integer(utility::from_string<i32>(value, overflow), 32);
			case type::I64:  return m_context.builder.create_signed_integer(utility::from_string<i64>(value, overflow), 64);
			case type::U8:   return m_context.builder.create_unsigned_integer(utility::from_string<u8>(value, overflow), 8);
			case type::U16:  return m_context.builder.create_unsigned_integer(utility::from_string<u16>(value, overflow), 16);
			case type::U32:  return m_context.builder.create_unsigned_integer(utility::from_string<u32>(value, overflow), 32);
			case type::U64:  return m_context.builder.create_unsigned_integer(utility::from_string<u64>(value, overflow), 64);
			// for cases when a numerical literal is implicitly converted to a bool (ie. "if(1)")
			case type::BOOL: {
				return m_context.builder.create_unsigned_integer(!utility::is_only_char(value, '0'), 8);
			}
			// for cases when a numerical literal is implicitly converted to a char (ie. "char c = 12")
			case type::CHAR: return m_context.builder.create_signed_integer(utility::from_string<i32>(value, overflow), 32);
			default: NOT_IMPLEMENTED();
		}

		return nullptr;
	}

	void ir_translator::copy_struct(handle<ir::node> destination, handle<ir::node> value, const type& struct_type, u16 base_offset) const {
		// copy over every struct member
		for (const auto& member : struct_type.get_struct_members()) {
			const u16 member_offset = struct_type.get_member_offset(member.get_member_identifier()) + base_offset;
			const u16 member_alignment = member.get_alignment();

			if (member.is_struct()) {
				copy_struct(destination, value, member, member_offset);
				continue;
			}

			const ir::data_type ir_type = detail::data_type_to_ir(member);

			// access the source value
			const handle<ir::node> source_access = m_context.builder.create_member_access(value, member_offset);
			const handle<ir::node> source_load = m_context.builder.create_load(source_access, ir_type, member_alignment, false);

			// access the member we want to store to
			const handle<ir::node> dest_access = m_context.builder.create_member_access(destination, member_offset);

			m_context.builder.create_store(dest_access, source_load, member_alignment, false);
		}
	}

	auto ir_translator::translate() -> utility::result<void> {
		for (const handle<ast::node>& top_level : m_context.syntax.ast.get_nodes()) {
			translate_node(top_level);
		}

		return SUCCESS;
	}
} // namespace sigma
