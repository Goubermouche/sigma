#include "type_checker.h"

#include <compiler/compiler/compilation_context.h>
#include <compiler/compiler/diagnostics.h>
#include <utility/string_helper.h>

namespace sigma {
	auto type_checker::type_check(backend_context& context) -> utility::result<void> {
		return type_checker(context).type_check();
	}

	type_checker::type_checker(backend_context& context) : m_context(context) {}

	auto type_checker::type_check() -> utility::result<void> {
		for (const handle<node>& top_level : m_context.syntax.ast.get_nodes()) {
			TRY(type_check_node(top_level, nullptr));
		}

		return SUCCESS;
	}

	auto type_checker::type_check_node(handle<node> ast_node, handle<node> parent, data_type expected) -> utility::result<data_type> {
		switch(ast_node->type) {
			// functions
			case node_type::FUNCTION_DECLARATION:  return type_check_function_declaration(ast_node);
			case node_type::FUNCTION_CALL:         return type_check_function_call(ast_node, parent, expected);

			case node_type::NAMESPACE_DECLARATION: return type_check_namespace_declaration(ast_node, expected);
			case node_type::EXPLICIT_CAST:         return type_check_explicit_cast(ast_node, parent, expected);
			case node_type::SIZEOF:                return type_check_sizeof(ast_node, parent, expected);

			// control flow
			case node_type::RETURN:                return type_check_return(ast_node, expected);
			case node_type::CONDITIONAL_BRANCH:    return type_check_conditional_branch(ast_node);
			case node_type::BRANCH:                return type_check_branch(ast_node);

			// variables
			case node_type::VARIABLE_DECLARATION:  return type_check_variable_declaration(ast_node);
			case node_type::VARIABLE_ACCESS:       return type_check_variable_access(ast_node, parent, expected);
			case node_type::VARIABLE_ASSIGNMENT:   return type_check_variable_assignment(ast_node);

			// binary operators
			case node_type::OPERATOR_ADD:
			case node_type::OPERATOR_SUBTRACT:
			case node_type::OPERATOR_MULTIPLY:
			case node_type::OPERATOR_DIVIDE:
			case node_type::OPERATOR_MODULO:       return type_check_binary_math_operator(ast_node, expected);


			// literals
			case node_type::NUMERICAL_LITERAL:     return type_check_numerical_literal(ast_node, expected);
			case node_type::CHARACTER_LITERAL:     return type_check_character_literal(ast_node, parent, expected);
			case node_type::STRING_LITERAL:        return type_check_string_literal(ast_node, parent, expected);
			case node_type::BOOL_LITERAL:          return type_check_bool_literal(ast_node, parent, expected);

			// unhandled node types
			default: PANIC("undefined type check for node '{}'", ast_node->type.to_string());
		}

		return data_type::create_unknown(); // unreachable
	}

	auto type_checker::type_check_namespace_declaration(handle<node> variable_node, data_type expected) -> utility::result<data_type> {
		SUPPRESS_C4100(expected);

		const ast_namespace& namespace_scope = variable_node->get<ast_namespace>();
		m_context.semantics.push_namespace(namespace_scope.identifier_key);

		// traverse inner statements (globals, functions, namespaces)
		for(const handle<node> statement : variable_node->children) {
			TRY(type_check_node(statement, variable_node));
		}

		m_context.semantics.pop_scope();
		return data_type::create_unknown(); // not used
	}

	auto type_checker::type_check_function_declaration(handle<node> function_node) -> utility::result<data_type> {
		const ast_function& function = function_node->get<ast_function>();

		// check if the function hasn't been declared before
		if(m_context.semantics.contains_function(function.signature)) {
			const std::string& identifier = m_context.syntax.strings.get(function.signature.identifier_key);
			return error::emit(error::code::FUNCTION_ALREADY_DECLARED, function_node->location, identifier);
		}

		// register the function
		m_context.semantics.pre_declare_local_function(function.signature);
		m_context.semantics.push_scope();

		// push temporaries for function parameters
		for(const named_data_type& parameter : function.signature.parameter_types) {
			auto& variable = m_context.semantics.pre_declare_variable(parameter.identifier_key, parameter.type);
			variable.flags |= variable::FUNCTION_PARAMETER | variable::LOCAL;
		}

		// type check inner statements
		for(const handle<node>& statement : function_node->children) {
			TRY(type_check_node(statement, function_node));
		}

		m_context.semantics.pop_scope();

		// this value won't be used
		return data_type::create_unknown();
	}

	auto type_checker::type_check_variable_declaration(handle<node> variable_node) -> utility::result<data_type> {
		const ast_variable& variable = variable_node->get<ast_variable>();

		// we cannot declare purely 'void' variables
		if(variable.type.is_void()) {
			const std::string& identifier_str = m_context.syntax.strings.get(variable.identifier_key);
			return error::emit(error::code::VOID_VARIABLE, variable_node->location, identifier_str);
		}

		// check, whether the variable has already been declared in the current context
		if(m_context.semantics.contains_variable(variable.identifier_key)) {
			const std::string& identifier_str = m_context.syntax.strings.get(variable.identifier_key);
			return error::emit(error::code::VARIABLE_ALREADY_DECLARED, variable_node->location, identifier_str);
		}

		// register the variable
		auto& declaration = m_context.semantics.pre_declare_variable(variable.identifier_key, variable.type);
		declaration.flags |= variable::LOCAL;

		// type check the assigned value
		if (variable_node->children.get_size() == 1) {
			TRY(type_check_node(variable_node->children[0], variable_node, variable.type));
		}

		// this value won't be used
		return data_type::create_unknown();
	}

	auto type_checker::type_check_function_call(handle<node> call_node, handle<node> parent, data_type expected) -> utility::result<data_type> {
		std::vector<data_type> params(call_node->children.get_size());
		ast_function_call& call = call_node->get<ast_function_call>();

		// type check all parameters and store their inherent type
		for(u64 i = 0; i < params.size(); ++i) {
			TRY(params[i], type_check_node(call_node->children[i], call_node));
			ASSERT(params[i].base_type != data_type::UNKNOWN, "unknown parameter type detected");
		}

		// at this point the function signature is empty, we gotta find a valid one
		TRY(call.signature, m_context.semantics.create_callee_signature(call_node, params));

		// we've found a valid function, upcast all provided parameters to the expected types
		u64 i = 0;
		for(; i < call.signature.parameter_types.get_size(); ++i) {
			const handle<node> target = call_node->children[i];
			implicit_type_cast(params[i], call.signature.parameter_types[i].type, call_node, target);
		}

		// type check var args
		for(; i < call_node->children.get_size(); ++i) {
			const handle<node> target = call_node->children[i];
			implicit_type_cast(params[i], data_type::create_var_arg_promote(), call_node, target);
		}

		ASSERT(i == call_node->children.get_size(), "invalid parameter count");

		// pass the return type along
		return implicit_type_cast(call.signature.return_type, expected, parent, call_node);
	}

	auto type_checker::type_check_return(handle<node> return_node, data_type expected) -> utility::result<data_type> {
		if (return_node->children.get_size() == 0) {
			// return an empty
			// verify that the parent function expects an empty return type
			if(expected != data_type(data_type::VOID, 0)) {
				return error::emit(error::code::UNEXPECTED_VOID_RET, return_node->location);
			}
		}
		else {
			// return a value
			TRY(type_check_node(return_node->children[0], return_node, expected));
		}

		// this value won't be used
		return data_type::create_unknown();
	}

	auto type_checker::type_check_conditional_branch(handle<node> branch_node) -> utility::result<data_type> {
		// type check the condition
		TRY(type_check_node(branch_node->children[0], branch_node, data_type(data_type::BOOL, 0)));

		// if children[1] exists, we have another branch node
		if (branch_node->children[1]) {
			// type check another conditional branch
			ASSERT(branch_node->children[1]->is_branch(), "cannot branch to a non-branch node");
			TRY(type_check_node(branch_node->children[1], nullptr));
		}

		m_context.semantics.push_scope();

		// type check inner statements
		for (u64 i = 2; i < branch_node->children.get_size(); ++i) {
			TRY(type_check_node(branch_node->children[i], branch_node));
		}

		m_context.semantics.pop_scope();

		// this value won't be used
		return data_type::create_unknown();
	}

	auto type_checker::type_check_branch(handle<node> branch_node) -> utility::result<data_type> {
		m_context.semantics.push_scope();

		// just type check all inner statements
		for (const handle<node>& statement : branch_node->children) {
			TRY(type_check_node(statement, branch_node));
		}

		m_context.semantics.pop_scope();

		// this value won't be used
		return data_type::create_unknown();
	}

	auto type_checker::type_check_binary_math_operator(handle<node> operator_node, data_type expected) -> utility::result<data_type> {
		// type check both operands
		TRY(const data_type left, type_check_node(operator_node->children[0], operator_node, expected));
		TRY(const data_type right, type_check_node(operator_node->children[1], operator_node, expected));

		// upcast both types
		const data_type larger_type = get_larger_type(left, right);

		TRY(implicit_type_cast(left, larger_type, operator_node, operator_node->children[0]));
		TRY(implicit_type_cast(right, larger_type, operator_node, operator_node->children[1]));

		return larger_type;
	}

	auto type_checker::type_check_numerical_literal(handle<node> literal_node, data_type expected) const -> utility::result<data_type> {
		auto& literal = literal_node->get<ast_literal>();

		// upcast to the expected type, without throwing warnings/errors
		literal.type = inherent_type_cast(literal.type, expected);

		const std::string& value_str = m_context.syntax.strings.get(literal.value_key);
		bool overflow = false;

		// check for type overflow
		switch (literal.type.base_type) {
			case data_type::I8: {
				const auto value = utility::detail::from_string<i8>(value_str, overflow);
				if (overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal_node->location, value_str, value, "i8"); }
				break;
			}
			case data_type::I16: {
				const auto value = utility::detail::from_string<i16>(value_str, overflow);
				if (overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal_node->location, value_str, value, "i16"); }
				break;
			}
			case data_type::I32: {
				const auto value = utility::detail::from_string<i32>(value_str, overflow);
				if(overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal_node->location, value_str, value, "i32"); }
				break;
			}
			case data_type::I64: {
				const auto value = utility::detail::from_string<i64>(value_str, overflow);
				if (overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal_node->location, value_str, value, "i64"); }
				break;
			}
			case data_type::U8: {
				const auto value = utility::detail::from_string<u8>(value_str, overflow);
				if (overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal_node->location, value_str, value, "u8"); }
				break;
			}
			case data_type::U16: {
				const auto value = utility::detail::from_string<u16>(value_str, overflow);
				if (overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal_node->location, value_str, value, "u16"); }
				break;
			}
			case data_type::U32: {
				const auto value = utility::detail::from_string<u32>(value_str, overflow);
				if(overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal_node->location, value_str, value, "u32"); }
				break;
			}
			case data_type::U64: {
				const auto value = utility::detail::from_string<u64>(value_str, overflow);
				if(overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal_node->location, value_str, value, "u64"); }
				break;
			}
			case data_type::BOOL: {
				warning::emit(warning::code::NUMERICAL_BOOL, literal_node->location);
				break;
			}
			default: NOT_IMPLEMENTED();
		}

		return literal.type;
	}

	auto type_checker::type_check_character_literal(handle<node> literal_node, handle<node> parent, data_type expected) const -> utility::result<data_type> {
		auto& literal = literal_node->get<ast_literal>();
		TRY(literal.type, implicit_type_cast(literal.type, expected, parent, literal_node));
		return literal.type;
	}

	auto type_checker::type_check_string_literal(handle<node> literal_node, handle<node> parent, data_type expected) const -> utility::result<data_type> {
		auto& literal = literal_node->get<ast_literal>();
		TRY(literal.type, implicit_type_cast(literal.type, expected, parent, literal_node));
		return literal.type;
	}

	auto type_checker::type_check_bool_literal(handle<node> literal_node, handle<node> parent, data_type expected) const -> utility::result<data_type> {
		TRY(const data_type type, implicit_type_cast(data_type::create_bool(), expected, parent, literal_node));
		return type;
	}

	auto type_checker::inherent_type_cast(data_type original_type, data_type target_type) -> data_type {
		if(target_type.is_unknown()) {
			return original_type;
		}

		return target_type;
	}

	auto type_checker::implicit_type_cast(data_type original_type, data_type target_type, handle<node> parent, handle<node> target) const -> utility::result<data_type> {
		if(target_type.is_unknown()) {
			return original_type;
		}

		if(target_type.is_promote()) {
			// promote the target type
			target_type = promote_type(original_type);
			// use the new promoted type and (up)cast to it
		}

		// no cast needed
		if(original_type == target_type) {
			return original_type;
		}

		// don't cast pointers implicitly
		if(original_type.is_pointer() || target_type.is_pointer()) {
			return error::emit(error::code::INVALID_IMPLICIT_CAST, target->location, original_type.to_string(), target_type.to_string());
		}

		// target type is known at this point, try to cast to it
		const u64 original_byte_width = original_type.get_byte_width();
		const u64 target_byte_width = target_type.get_byte_width();

		// no cast needed, probably a sign diff
		if(original_byte_width == target_byte_width) {
			warning::emit(warning::code::IMPLICIT_CAST, target->location, original_type.to_string(), target_type.to_string());
			return original_type;
		}

		ASSERT(parent, "parent is nulltr");

		// insert a cast node between the target node and its parent node
		//  parent         parent
		//    |              |
		//    |      -->    cast
		//    |              |
		// target     	target

		const bool truncate = original_byte_width > target_byte_width;
		const handle<node> cast_node = m_context.syntax.ast.create_node<ast_cast>(truncate ? node_type::CAST_TRUNCATE : node_type::CAST_EXTEND, 1, nullptr);
		ast_cast& cast = cast_node->get<ast_cast>();
		cast.original_type = original_type;
		cast.target_type = target_type;

		// find the 'target' node in the parent
		u64 index_in_parent = 0;
		for(; index_in_parent < parent->children.get_size(); index_in_parent++) {
			if(parent->children[index_in_parent] == target) {
				break;
			}
		}

		// replace this node by the relevant cast
		parent->children[index_in_parent] = cast_node;
		cast_node->children[0] = target;

		warning::emit(
			truncate ? warning::code::IMPLICIT_TRUNCATION_CAST : warning::code::IMPLICIT_EXTENSION_CAST,
			target->location,
			original_type.to_string(),
			target_type.to_string()
		);

		return target_type;
	}

	auto type_checker::explicit_type_cast(data_type original_type, data_type target_type, handle<node> target) const -> utility::result<void> {
		if(original_type.pointer_level != target_type.pointer_level) {
			return error::emit(error::code::INCOMPATIBLE_EXPLICIT_CAST, target->location, original_type.to_string(), target_type.to_string());
		}

		return SUCCESS;
	}

	auto type_checker::type_check_variable_access(handle<node> access_node, handle<node> parent, data_type expected) const -> utility::result<data_type> {
		auto& access = access_node->get<ast_variable>();

		// locate the variable
		TRY(const auto variable, m_context.semantics.find_variable(access.identifier_key));

		// check if the variable exists
		if(variable == nullptr) {
			const std::string& identifier_str = m_context.syntax.strings.get(access.identifier_key);
			return error::emit(error::code::UNKNOWN_VARIABLE, access_node->location, identifier_str);
		}

		// default to the declared type
		TRY(access.type, implicit_type_cast(variable->type, expected, parent, access_node));
		return access.type; // return the type checked value
	}

	auto type_checker::type_check_variable_assignment(handle<node> assignment_node) -> utility::result<data_type> {
		// locate the variable we're assigning to
		const handle<node> variable_node = assignment_node->children[0];
		const auto& variable = variable_node->get<ast_variable>();

		TRY(const auto declaration, m_context.semantics.find_variable(variable.identifier_key));

		// check if the variable exists
		if (declaration == nullptr) {
			const std::string& identifier_str = m_context.syntax.strings.get(variable.identifier_key);
			return error::emit(error::code::UNKNOWN_VARIABLE_ASSIGN, variable_node->location, identifier_str);
		}

		// type check the assigned value against the declared type
		TRY(type_check_node(assignment_node->children[1], assignment_node, declaration->type));

		// this value won't be used
		return data_type::create_unknown();
	}

	auto type_checker::type_check_explicit_cast(handle<node> cast_node, handle<node> parent, data_type expected) -> utility::result<data_type> {
		ast_cast& cast = cast_node->get<ast_cast>();

		TRY(cast.original_type, type_check_node(cast_node->children[0], cast_node, data_type::create_unknown()));
		TRY(explicit_type_cast(cast.original_type, cast.target_type, cast_node));

		// upcast the result, if necessary, just a sanity check
		return implicit_type_cast(cast.target_type, expected, parent, cast_node);
	}

	auto type_checker::type_check_sizeof(handle<node> sizeof_node, handle<node> parent, data_type expected) -> utility::result<data_type> {
		// upcast to the expected type, without throwing warnings/errors
		return implicit_type_cast(data_type::create_u64(), expected, parent, sizeof_node);
	}
} // namespace sigma
