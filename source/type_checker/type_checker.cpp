#include "type_checker.h"
#include <compiler/compiler/compilation_context.h>
#include <compiler/compiler/diagnostics.h>

namespace sigma {
	auto type_checker::type_check(backend_context& context) -> utility::result<void> {
		return type_checker(context).type_check();
	}

	type_checker::type_checker(backend_context& context) : m_context(context) {}

	auto type_checker::type_check() -> utility::result<void> {
		for(const ast_node& top_level : m_context.syntax.ast.get_nodes()) {
			// type check all nodes
			TRY(type_check_node(top_level, nullptr));
		}

		return SUCCESS;
	}

	auto type_checker::type_check_node(ast_node target, ast_node parent, type expected) -> type_check_result {
		switch(target->type) {
			// declarations
			case ast::node_type::NAMESPACE_DECLARATION:          return type_check_namespace_declaration(target);
			case ast::node_type::FUNCTION_DECLARATION:           return type_check_function_declaration(target);
			case ast::node_type::VARIABLE_DECLARATION:           return type_check_variable_declaration(target);
			case ast::node_type::STRUCT_DECLARATION:             return type_check_struct_declaration(target);

			// literals
			case ast::node_type::NUMERICAL_LITERAL:              return type_check_numerical_literal(target, expected);
			case ast::node_type::CHARACTER_LITERAL:              return type_check_character_literal(target, parent, expected);
			case ast::node_type::STRING_LITERAL:                 return type_check_string_literal(target, parent, expected);
			case ast::node_type::BOOL_LITERAL:                   return type_check_bool_literal(target, parent, expected);

			// expressions
			case ast::node_type::OPERATOR_ADD:
			case ast::node_type::OPERATOR_SUBTRACT:
			case ast::node_type::OPERATOR_MULTIPLY:
			case ast::node_type::OPERATOR_DIVIDE:
			case ast::node_type::OPERATOR_MODULO:                return type_check_binary_math_operator(target, expected);
			case ast::node_type::OPERATOR_GREATER_THAN_OR_EQUAL:
			case ast::node_type::OPERATOR_LESS_THAN_OR_EQUAL:
			case ast::node_type::OPERATOR_GREATER_THAN:
			case ast::node_type::OPERATOR_NOT_EQUAL:
			case ast::node_type::OPERATOR_LESS_THAN:
			case ast::node_type::OPERATOR_EQUAL:                 return type_check_binary_comparison_operator(target, parent, expected);
			case ast::node_type::OPERATOR_CONJUNCTION:           
			case ast::node_type::OPERATOR_DISJUNCTION:           return type_check_predicate_operator(target, parent, expected);
			case ast::node_type::OPERATOR_LOGICAL_NOT:           return type_check_not_operator(target, parent, expected);

			// statements
			case ast::node_type::RETURN:                         return type_check_return(target);
			case ast::node_type::CONDITIONAL_BRANCH:             return type_check_conditional_branch(target);
			case ast::node_type::BRANCH:                         return type_check_branch(target);

			// loads / stores
			case ast::node_type::VARIABLE_ACCESS:                return type_check_variable_access(target, parent, expected);
			case ast::node_type::ARRAY_ACCESS:                   return type_check_array_access(target, parent, expected);
			case ast::node_type::LOCAL_MEMBER_ACCESS:            return type_check_local_member_access(target, parent, expected);
			case ast::node_type::LOAD:                           return type_check_load(target, expected);
			case ast::node_type::STORE:                          return type_check_store(target);

			// other
			case ast::node_type::FUNCTION_CALL:                  return type_check_function_call(target, parent, expected);
			// since implicit casts are not type checked we can interpret these casts a being explicit
			case ast::node_type::CAST:                           return type_check_explicit_cast(target, parent, expected);
			case ast::node_type::ALIGNOF:                        return type_check_alignof(target, parent, expected);
			case ast::node_type::SIZEOF:                         return type_check_sizeof(target, parent, expected);

			// unhandled node types
			default: PANIC("undefined type check for node '{}'", target->type.to_string());
		}

		return type::create_unknown(); // unreachable
	}

	auto type_checker::type_check_namespace_declaration(ast_node declaration) -> type_check_result {
		const ast::named_expression& namespace_scope = declaration->get<ast::named_expression>();
		m_context.semantics.push_namespace(namespace_scope.key);

		// traverse inner statements (globals, functions, namespaces)
		for(const ast_node statement : declaration->children) {
			TRY(type_check_node(statement, declaration));
		}

		m_context.semantics.pop_scope();
		return type::create_unknown(); // not used
	}

	auto type_checker::type_check_function_declaration(ast_node declaration) -> type_check_result {
		ast::function& function = declaration->get<ast::function>();

		// check if the function hasn't been declared before
		if(m_context.semantics.contains_function(function.signature)) {
			const std::string& identifier = m_context.syntax.strings.get(function.signature.identifier_key);
			return error::emit(error::code::FUNCTION_ALREADY_DECLARED, declaration->location, identifier);
		}

		// register the function
		m_context.semantics.pre_declare_local_function(function.signature);
		m_context.semantics.push_scope(scope::control_type::UNCONDITIONAL);
		m_current_function = function.signature;

		// push temporaries for function parameters
		for(named_data_type& parameter : function.signature.parameter_types) {
			TRY(m_context.semantics.resolve_type(parameter.type, declaration->location));

			auto& variable = m_context.semantics.pre_declare_variable(parameter.identifier_key, parameter.type);
			variable.flags |= variable::FUNCTION_PARAMETER | variable::LOCAL;
		}

		// type check inner statements
		for(const ast_node& statement : declaration->children) {
			TRY(type_check_node(statement, declaration));
		}

		TRY(m_context.semantics.verify_control_flow(declaration));
		m_context.semantics.pop_scope();

		// this value won't be used
		return type::create_unknown();
	}

	auto type_checker::type_check_variable_declaration(ast_node declaration) const -> type_check_result {
		ast::named_type_expression& variable = declaration->get<ast::named_type_expression>();
		TRY(m_context.semantics.resolve_type(variable.type, declaration->location));

		// we cannot declare purely 'void' variables
		if(variable.type.is_pure_void()) {
			const std::string& identifier_str = m_context.syntax.strings.get(variable.key);
			return error::emit(error::code::VOID_VARIABLE, declaration->location, identifier_str);
		}

		// check, whether the variable has already been declared in the current context
		if(m_context.semantics.contains_variable(variable.key)) {
			const std::string& identifier_str = m_context.syntax.strings.get(variable.key);
			return error::emit(error::code::VARIABLE_ALREADY_DECLARED, declaration->location, identifier_str);
		}

		// register the variable
		auto& var = m_context.semantics.pre_declare_variable(variable.key, variable.type);
		var.flags |= variable::LOCAL; // mark it as a local variable

		return variable.type;
	}

	auto type_checker::type_check_function_call(ast_node call, ast_node parent, type expected) -> type_check_result {
		std::vector<type> parameters(call->children.get_size());
		ast::function_call& function_call = call->get<ast::function_call>();

		// type check all parameters and store their inherent type
		for(u64 i = 0; i < parameters.size(); ++i) {
			TRY(parameters[i], type_check_node(call->children[i], call));
			ASSERT(!parameters[i].is_unknown(), "unknown parameter type detected");
		}

		// at this point the function signature is empty, we gotta find a valid one
		TRY(function_call.signature, m_context.semantics.find_callee_signature(call, parameters));

		// we've found a valid function, upcast all provided parameters to the expected types
		// upcast regular parameters
		u64 i = 0;
		for(; i < function_call.signature.parameter_types.get_size(); ++i) {
			const ast_node target = call->children[i];
			const type type = function_call.signature.parameter_types[i].type;

			TRY(implicit_type_cast(parameters[i], type, call, target));
		}

		// upcast var parameters
		for(; i < call->children.get_size(); ++i) {
			const ast_node target = call->children[i];

			TRY(implicit_type_cast(parameters[i], type::create_promote(), call, target));
		}

		ASSERT(i == call->children.get_size(), "invalid parameter count");

		// pass the return type along
		return implicit_type_cast(function_call.signature.return_type, expected, parent, call);
	}

	auto type_checker::type_check_return(ast_node statement) -> type_check_result {
		if (statement->children.get_size() == 0) {
			// return an empty
			// verify that the parent function expects an empty return type
			if(!m_current_function.return_type.is_pure_void()) {
				return error::emit(error::code::UNEXPECTED_EMPTY_VOID_RET, statement->location);
			}
		}
		else {
			// verify that the parent function expects a return type
			if(m_current_function.return_type.is_pure_void()) {
				return error::emit(error::code::UNEXPECTED_NON_EMPTY_VOID_RET, statement->location);
			}

			// return a value
			TRY(type_check_node(statement->children[0], statement, m_current_function.return_type));
			m_context.semantics.declare_return();
		}

		// this value won't be used
		return type::create_unknown();
	}

	auto type_checker::type_check_conditional_branch(ast_node branch) -> type_check_result {
		// type check the condition
		TRY(type_check_node(branch->children[0], branch, type::create_bool()));

		// if children[1] exists, we have a child branch node
		if (branch->children[1]) {
			// type check another conditional branch
			ASSERT(branch->children[1]->is_branch(), "cannot branch to a non-branch node");
			TRY(type_check_node(branch->children[1], nullptr));
		}

		m_context.semantics.push_scope(scope::control_type::CONDITIONAL);

		// type check inner statements
		for (u16 i = 2; i < branch->children.get_size(); ++i) {
			TRY(type_check_node(branch->children[i], branch));
		}

		m_context.semantics.pop_scope();

		// this value won't be used
		return type::create_unknown();
	}

	auto type_checker::type_check_branch(ast_node branch) -> type_check_result {
		m_context.semantics.push_scope(scope::control_type::UNCONDITIONAL);

		// just type check all inner statements
		for (const ast_node& statement : branch->children) {
			TRY(type_check_node(statement, branch));
		}

		m_context.semantics.pop_scope();

		// this value won't be used
		return type::create_unknown();
	}

	auto type_checker::type_check_binary_math_operator(ast_node binop, type expected) -> type_check_result {
		// type check both operands
		TRY(const type left, type_check_node(binop->children[0], binop, expected)); // left
		TRY(const type right, type_check_node(binop->children[1], binop, expected)); // right

		// upcast both types
		const type larger_type = detail::get_larger_type(left, right);

		TRY(implicit_type_cast(left, larger_type, binop, binop->children[0]));
		TRY(implicit_type_cast(right, larger_type, binop, binop->children[1]));

		return larger_type;
	}

	auto type_checker::type_check_predicate_operator(ast_node binop, ast_node parent, type expected) -> type_check_result {
		// type check both operands
		TRY(type_check_node(binop->children[0], binop, type::create_bool())); // left
		TRY(type_check_node(binop->children[1], binop, type::create_bool())); // right

		return implicit_type_cast(type::create_bool(), expected, parent, binop);
	}

	auto type_checker::type_check_binary_comparison_operator(ast_node binop, ast_node parent, type expected) -> type_check_result {
		// type check both operands
		TRY(const type left, type_check_node(binop->children[0], binop)); // left
		TRY(const type right, type_check_node(binop->children[1], binop)); // right

		// upcast both types
		const type larger_type = detail::get_larger_type(left, right);

		TRY(implicit_type_cast(left, larger_type, binop, binop->children[0]));
		TRY(implicit_type_cast(right, larger_type, binop, binop->children[1]));

		ast::comparison_expression& expression = binop->get<ast::comparison_expression>();

		// determine the type of our comparison op
		if(larger_type.is_pointer()) {
			expression.type = ast::comparison_expression::type::POINTER;
		}
		else if(larger_type.is_floating_point()) {
			expression.type = ast::comparison_expression::type::FLOATING_POINT;
		}
		else if(larger_type.is_signed()) {
			expression.type = ast::comparison_expression::type::INTEGRAL_SIGNED;
		}
		else {
			expression.type = ast::comparison_expression::type::INTEGRAL_UNSIGNED;
		}

		return implicit_type_cast(type::create_bool(), expected, parent, binop);
	}

	auto type_checker::type_check_array_access(ast_node access, ast_node parent, type expected) -> type_check_result {
		TRY(const type base_type, type_check_node(access->children[0], access));
		TRY(type_check_node(access->children[1], access, type::create_u64()));

		access->get<ast::type_expression>().type = base_type;
		const type accessed_type = base_type.dereference(1);

		return implicit_type_cast(accessed_type, expected, parent, access);
	}

	auto type_checker::type_check_local_member_access(ast_node access, ast_node parent, type expected) -> type_check_result {
		auto& expression = access->get<ast::named_type_expression>();

		// type check the storage location
		TRY(const type base_type, type_check_node(access->children[0], access));

		for(const type& member : base_type.get_struct_members()) {
			// find a matching member
			if(member.get_member_identifier() == expression.key) {
				TRY(expression.type, implicit_type_cast(member, expected, parent, access));
				return expression.type;
			}
		}

		// no member was found
		const std::string& identifier = m_context.syntax.strings.get(expression.key);
		return error::emit(error::code::UNKNOWN_STRUCT_MEMBER, access->location, identifier);
	}

	auto type_checker::type_check_load(ast_node load, type expected) -> type_check_result {
		// type check the loaded node
		TRY(const type load_type, type_check_node(load->children[0], load, expected));

		// assign the loaded type
		load->get<ast::type_expression>().type = load_type;

		return load_type;
	}

	auto type_checker::type_check_numerical_literal(ast_node literal, type expected) const -> type_check_result {
		// upcast to the expected type, without throwing warnings/errors
		auto& expression = literal->get<ast::named_type_expression>();
		expression.type = inherent_type_cast(expression.type, expected);

		const std::string& value_str = m_context.syntax.strings.get(expression.key);
		bool overflow = false;

		// check for type overflow
		switch (expression.type.get_kind()) {
			case type::I8: {
				const auto value = utility::from_string<i8>(value_str, overflow);
				if (overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal->location, value_str, value, "i8"); }
				break;
			}
			case type::I16: {
				const auto value = utility::from_string<i16>(value_str, overflow);
				if (overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal->location, value_str, value, "i16"); }
				break;
			}
			case type::I32: {
				const auto value = utility::from_string<i32>(value_str, overflow);
				if(overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal->location, value_str, value, "i32"); }
				break;
			}
			case type::I64: {
				const auto value = utility::from_string<i64>(value_str, overflow);
				if (overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal->location, value_str, value, "i64"); }
				break;
			}
			case type::U8: {
				const auto value = utility::from_string<u8>(value_str, overflow);
				if (overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal->location, value_str, value, "u8"); }
				break;
			}
			case type::U16: {
				const auto value = utility::from_string<u16>(value_str, overflow);
				if (overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal->location, value_str, value, "u16"); }
				break;
			}
			case type::U32: {
				const auto value = utility::from_string<u32>(value_str, overflow);
				if(overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal->location, value_str, value, "u32"); }
				break;
			}
			case type::U64: {
				const auto value = utility::from_string<u64>(value_str, overflow);
				if(overflow) { warning::emit(warning::code::LITERAL_OVERFLOW, literal->location, value_str, value, "u64"); }
				break;
			}
			case type::BOOL: {
				warning::emit(warning::code::NUMERICAL_BOOL, literal->location);
				break;
			}
			case type::CHAR: {
				warning::emit(warning::code::NUMERICAL_CHAR, literal->location);
				break;
			}
			default: NOT_IMPLEMENTED();
		}

		return expression.type;
	}

	auto type_checker::type_check_character_literal(ast_node literal, ast_node parent, type expected) const -> type_check_result {
		auto& expression = literal->get<ast::named_type_expression>();
		TRY(expression.type, implicit_type_cast(expression.type, expected, parent, literal));
		return expression.type;
	}

	auto type_checker::type_check_string_literal(ast_node literal, ast_node parent, type expected) const -> type_check_result {
		auto& expression = literal->get<ast::named_type_expression>();
		TRY(expression.type, implicit_type_cast(expression.type, expected, parent, literal));
		return expression.type;
	}

	auto type_checker::type_check_struct_declaration(ast_node declaration) const -> type_check_result {
		auto& expression = declaration->get<ast::named_type_expression>();

		// verify that no two members of the struct have the same identifier
		auto& members = expression.type.get_struct_members();

		for (u64 i = 0; i < members.get_size(); ++i) {
			for (u64 j = i + 1; j < members.get_size(); ++j) {
				if(members[i].get_member_identifier() == members[j].get_member_identifier()) {
					const std::string& identifier = m_context.syntax.strings.get(members[i].get_member_identifier());
					return error::emit(error::code::DUPLICATE_STRUCT_IDENTIFIER, declaration->location, identifier);
				}
			}
		}

		// resolve inner types
		for(type& member : members) {
			TRY(m_context.semantics.resolve_type(member, declaration->location));
		}

		TRY(m_context.semantics.declare_struct(declaration));
		return type::create_unknown(); // not used
	}

	auto type_checker::type_check_bool_literal(ast_node literal, ast_node parent, type expected) const -> type_check_result {
		// no need to check this, just cast it
		return implicit_type_cast(type::create_bool(), expected, parent, literal);
	}

	auto type_checker::inherent_type_cast(type original_type, type target_type) -> type {
		if(target_type.is_unknown()) {
			return original_type;
		}

		return target_type;
	}

	auto type_checker::implicit_type_cast(type original_type, type target_type, ast_node parent, ast_node target) const -> type_check_result {
		if(target_type.is_unknown()) {
			return original_type;
		}

		if(original_type.is_pure_void() || target_type.is_pure_void()) {
			return error::emit(error::code::INVALID_VOID, target->location);
		}

		if(target_type.is_promote()) {
			// promote the target type
			target_type = detail::promote_type(original_type);
			// use the new promoted type and (up)cast to it
		}

		// types are the same
		if(original_type == target_type) {
			return original_type;
		}

		// pointer casting
		if(original_type.is_pointer()) {
			if(target_type.is_void_pointer()) {
				// allow implicit casting to void*
				return target_type;
			}

			// invalid implicit cast
			return error::emit(
				error::code::INVALID_IMPLICIT_CAST, 
				target->location, 
				original_type.to_string(),
				target_type.to_string()
			);
		}
		else {
			// check that we're not casting from a non-pointer to a pointer
			if (target_type.is_pointer()) {
				// invalid cast
				return error::emit(
					error::code::INVALID_CAST,
					target->location,
					original_type.to_string(),
					target_type.to_string()
				);
			}
		}

		// target type is known at this point, try to cast to it
		const u16 original_byte_width = original_type.get_size();
		const u16 target_byte_width = target_type.get_size();

		// no cast needed, probably a sign diff
		if(original_byte_width == target_byte_width) {
			warning::emit(
				warning::code::IMPLICIT_CAST,
				target->location, 
				original_type.to_string(),
				target_type.to_string()
			);

			return target_type;
		}

		ASSERT(parent, "invalid parent detected");

		// insert a cast node between the target node and the target's parent node
		const bool truncate = original_byte_width > target_byte_width;

		// create the cast node
		const ast_node cast_node = m_context.syntax.ast.create_node<ast::cast>(ast::node_type::CAST, 1, nullptr);

		// assign cast info
		ast::cast& cast = cast_node->get<ast::cast>();
		cast.original_type = original_type;
		cast.target_type = target_type;

		// find the 'target' node in the parent
		u16 index_in_parent = 0;
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

	auto type_checker::type_check_variable_access(ast_node access, ast_node parent, type expected) const -> type_check_result {
		auto& expression = access->get<ast::named_type_expression>();

		// locate the variable
		TRY(const auto variable, m_context.semantics.find_variable(expression.key));

		// check if the variable exists
		if(variable == nullptr) {
			const std::string& identifier = m_context.syntax.strings.get(expression.key);
			return error::emit(error::code::UNKNOWN_VARIABLE, access->location, identifier);
		}

		// default to the expected type
		TRY(expression.type, implicit_type_cast(variable->type, expected, parent, access));
		return expression.type; // return the type checked value
	}

	auto type_checker::type_check_store(ast_node store) -> type_check_result {
		// type check the destination
		const ast_node destination = store->children[0];
		TRY(const type destination_type, type_check_node(destination, store));

		// type check the assigned value against the destination type
		TRY(type_check_node(store->children[1], store, destination_type));

		// this value won't be used
		return type::create_unknown();
	}

	auto type_checker::type_check_explicit_cast(ast_node cast, ast_node parent, type expected) -> type_check_result {
		ast::cast& value = cast->get<ast::cast>();

		// type check the value we're casting
		TRY(value.original_type, type_check_node(cast->children[0], cast));
		TRY(m_context.semantics.resolve_type(value.target_type, cast->location));

		const type original = value.original_type;
		const type target = value.target_type;

		// verify the cast integrity
		if (!original.is_pointer() && target.is_pointer()) {
			// don't allow non-pointers to become pointers
				// invalid cast
			return error::emit(
				error::code::INVALID_CAST,
				cast->location,
				original.to_string(),
				target.to_string()
			);
		}

		// upcast the result, if necessary, just a sanity check
		return implicit_type_cast(value.target_type, expected, parent, cast);
	}

	auto type_checker::type_check_alignof(ast_node alignof_node, ast_node parent, type expected) const -> type_check_result {
		// upcast to the expected type, without throwing warnings/errors
		TRY(m_context.semantics.resolve_type(alignof_node->get<ast::type_expression>().type, alignof_node->location));
		return implicit_type_cast(type::create_u64(), expected, parent, alignof_node);
	}

	auto type_checker::type_check_sizeof(ast_node sizeof_node, ast_node parent, type expected) const -> type_check_result {
		// upcast to the expected type, without throwing warnings/errors
		TRY(m_context.semantics.resolve_type(sizeof_node->get<ast::type_expression>().type, sizeof_node->location));
		return implicit_type_cast(type::create_u64(), expected, parent, sizeof_node);
	}

	auto type_checker::type_check_not_operator(ast_node op, ast_node parent, type expected) -> type_check_result {
		// type check the negated expression
		TRY(const type expression_type, type_check_node(op->children[0], op, type::create_bool()));

		// upcast, just in case, more of a sanity check
		return implicit_type_cast(expression_type, expected, parent, op);
	}
} // namespace sigma
