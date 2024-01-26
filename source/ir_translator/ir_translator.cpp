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

	handle<ir::node> ir_translator::translate_node(handle<node> ast_node) {
		switch (ast_node->type) {
			case node_type::FUNCTION_DECLARATION:  translate_function_declaration(ast_node); break;
			case node_type::FUNCTION_CALL:         return translate_function_call(ast_node);

			case node_type::NAMESPACE_DECLARATION: translate_namespace_declaration(ast_node); break;
			case node_type::SIZEOF:                return translate_sizeof(ast_node);

			// flow control
			case node_type::RETURN:                translate_return(ast_node); break;
			case node_type::CONDITIONAL_BRANCH:    translate_conditional_branch(ast_node, nullptr); break;

			// variables
			case node_type::VARIABLE_DECLARATION:  translate_variable_declaration(ast_node); break;
			case node_type::VARIABLE_ASSIGNMENT:   return translate_variable_assignment(ast_node);

			case node_type::LOAD:                  return translate_load(ast_node);

			case node_type::VARIABLE_ACCESS:       return translate_variable_access(ast_node);
			case node_type::ARRAY_ACCESS:          return translate_array_access(ast_node);

			// operators:
			case node_type::OPERATOR_ADD:
			case node_type::OPERATOR_SUBTRACT:
			case node_type::OPERATOR_MULTIPLY:
			case node_type::OPERATOR_DIVIDE:
			case node_type::OPERATOR_MODULO:       return translate_binary_math_operator(ast_node);

			case node_type::CAST:                  return translate_cast(ast_node);

			// literals
			case node_type::NUMERICAL_LITERAL:     return translate_numerical_literal(ast_node);
			case node_type::CHARACTER_LITERAL:     return translate_character_literal(ast_node);
			case node_type::STRING_LITERAL:        return translate_string_literal(ast_node);
			case node_type::BOOL_LITERAL:          return translate_bool_literal(ast_node);
			default: PANIC("irgen for node '{}' is not implemented", ast_node->type.to_string());
		}

		return nullptr;
	}

	void ir_translator::translate_function_declaration(handle<node> function_node) {
		const ast_function& function = function_node->get<ast_function>();

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
		for (const handle<node>& statement : function_node->children) {
			translate_node(statement);
		}

		m_context.semantics.declare_implicit_return();
		m_context.semantics.trace_pop_scope();
	}

	void ir_translator::translate_variable_declaration(handle<node> variable_node) {
		const auto& prop = variable_node->get<ast_variable>();
		const u16 byte_width = prop.type.get_byte_width();
		const handle<ir::node> local = m_context.semantics.declare_variable(prop.identifier_key, byte_width, byte_width);

		if (variable_node->children.get_size() == 1) {
			const handle<ir::node> expression = translate_node(variable_node->children[0]);
			m_context.builder.create_store(local, expression, byte_width, false);
		}
		else if (prop.type.base_type == data_type::BOOL) {
			// boolean values should default to false
			m_context.builder.create_store(local, m_context.builder.create_bool(false), byte_width, false);
		}
	}

	void ir_translator::translate_namespace_declaration(handle<node> namespace_node) {
		m_context.semantics.trace_push_scope();

		for(const handle<node> statement : namespace_node->children) {
			translate_node(statement);
		}

		m_context.semantics.trace_pop_scope();
	}

	void ir_translator::translate_return(handle<node> return_node) {
		if (return_node->children.get_size() == 0) {
			m_context.builder.create_return({}); // TODO: maybe this should return a VOID_TY?
		}
		else {
			m_context.builder.create_return({ translate_node(return_node->children[0]) });
		}
	}

	auto ir_translator::translate_sizeof(handle<node> sizeof_node) const -> handle<ir::node> {
		const ast_sizeof& sizeof_value = sizeof_node->get<ast_sizeof>();
		const u16 byte_width = sizeof_value.type.get_byte_width();

		return m_context.builder.create_unsigned_integer(byte_width, 64);
	}

	void ir_translator::translate_conditional_branch(handle<node> branch_node, handle<ir::node> end_control) {
		const handle<ir::node> true_control = m_context.builder.create_region();
		const handle<ir::node> false_control = m_context.builder.create_region();
		end_control = end_control ? end_control : m_context.builder.create_region();

		// translate the condition
		const handle<ir::node> condition = translate_node(branch_node->children[0]);

		// check if there is a successor branch node
		if (const handle<node> successor = branch_node->children[1]) {
			// successor node
			m_context.builder.create_conditional_branch(condition, true_control, false_control);
			m_context.builder.set_control(false_control);

			[[likely]]
			if (successor->type == node_type::CONDITIONAL_BRANCH) {
				// else if
				translate_conditional_branch(successor, end_control);
			}
			else if (successor->type == node_type::BRANCH) {
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

	void ir_translator::translate_branch(handle<node> branch_node, handle<ir::node> exit_control) {
		m_context.semantics.trace_push_scope();

		for (const handle<node>& statement : branch_node->children) {
			translate_node(statement);
		}

		m_context.semantics.trace_pop_scope();
		m_context.builder.create_branch(exit_control);
	}

	auto ir_translator::translate_array_access(handle<node> access_node) -> handle<ir::node> {
		ASSERT(access_node->children.get_size() - 1 == 1, "implement support for accessing multiple array levels");

		// get the storage location
		const handle<ir::node> base = translate_node(access_node->children[0]);
		const handle<ir::node> index = translate_node(access_node->children[1]);

		// stride calculated by the type checker
		const i64 stride = access_node->get<ast_array_access>().stride;

		return m_context.builder.create_array_access(base, index, stride);
	}

	auto ir_translator::translate_numerical_literal(handle<node> numerical_literal_node) const -> handle<ir::node> {
		return literal_to_ir(numerical_literal_node->get<ast_literal>());
	}

	auto ir_translator::translate_character_literal(handle<node> character_literal_node) const -> handle<ir::node> {
		const std::string& value = m_context.syntax.strings.get(character_literal_node->get<ast_literal>().value_key);
		ASSERT(value.size() == 1, "invalid char literal length");
		return m_context.builder.create_signed_integer(value[0], 32);
	}

	auto ir_translator::translate_string_literal(handle<node> string_literal_node) const -> handle<ir::node> {
		const std::string& value = m_context.syntax.strings.get(string_literal_node->get<ast_literal>().value_key);
		return m_context.builder.create_string(value);
	}

	auto ir_translator::translate_bool_literal(handle<node> bool_literal_node) const -> handle<ir::node> {
		const auto& prop = bool_literal_node->get<ast_bool_literal>();
		return m_context.builder.create_bool(prop.value);
	}

	auto ir_translator::translate_binary_math_operator(handle<node> operator_node) -> handle<ir::node> {
		const handle<ir::node> left  = translate_node(operator_node->children[0]);
		const handle<ir::node> right = translate_node(operator_node->children[1]);

		switch (operator_node->type) {
			case node_type::OPERATOR_ADD:      return m_context.builder.create_add(left, right);
			case node_type::OPERATOR_SUBTRACT: return m_context.builder.create_sub(left, right);
			case node_type::OPERATOR_MULTIPLY: return m_context.builder.create_mul(left, right);
			//case node_type::OPERATOR_DIVIDE:   
			//case node_type::OPERATOR_MODULO:
			default: PANIC("unexpected node type '{}' received", operator_node->type.to_string());
		}

		return nullptr; // unreachable
	}

	auto ir_translator::translate_cast(handle<node> cast_node) -> handle<ir::node> {
		const ast_cast& cast = cast_node->get<ast_cast>();

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

	auto ir_translator::translate_function_call(handle<node> call_node) -> handle<ir::node> {
		const ast_function_call& callee = call_node->get<ast_function_call>();

		std::vector<handle<ir::node>> parameters;
		parameters.reserve(call_node->children.get_size());

		for (const handle<node>& parameter : call_node->children) {
			parameters.push_back(translate_node(parameter));
		}

		const handle<ir::node> call_result = m_context.semantics.create_call(callee.signature, callee.namespaces, parameters);
		return call_result;
	}

	auto ir_translator::translate_load(handle<node> load_node) -> handle<ir::node> {
		// translate the target value
		const handle<ir::node> value_to_load = translate_node(load_node->children[0]);

		// get the type and alignment of the load
		const data_type& type_to_load = load_node->get<ast_load>().type;
		const ir::data_type ir_type = detail::data_type_to_ir(type_to_load);
		const u16 alignment = type_to_load.get_byte_width();

		// create the load operation
		return m_context.builder.create_load(value_to_load, ir_type, alignment, false);
	}

	auto ir_translator::translate_variable_access(handle<node> access_node) const -> handle<ir::node> {
		const auto& accessed_variable = access_node->get<ast_variable>();

		// just return the variable value, at this point everything was handled by the type checker
		return m_context.semantics.get_variable(accessed_variable.identifier_key)->value;
	}

	auto ir_translator::translate_variable_assignment(handle<node> assignment_node) -> handle<ir::node> {
		const auto& variable = assignment_node->children[0]->get<ast_variable>();
		const u16 alignment = variable.type.get_byte_width();

		// get the value we want to store
		const handle<ir::node> value_to_store = translate_node(assignment_node->children[1]);

		// get the storage location
		const handle<node> storage_node = assignment_node->children[0];
		const handle<ir::node> storage = translate_node(storage_node);

		m_context.builder.create_store(storage, value_to_store, alignment, false);
		return nullptr;
	}

	auto ir_translator::literal_to_ir(const ast_literal& literal) const -> handle<ir::node> {
		const std::string& value = m_context.syntax.strings.get(literal.value_key);

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
		for (const handle<node>& top_level : m_context.syntax.ast.get_nodes()) {
			translate_node(top_level);
		}

		return SUCCESS;
	}
} // namespace sigma
