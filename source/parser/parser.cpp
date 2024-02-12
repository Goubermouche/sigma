#include "parser.h"
#include <compiler/compiler/compilation_context.h>
#include <compiler/compiler/diagnostics.h>

/**
 * \brief Checks if the current token is the expected \b __token, if a mismatch
 * is detected, an error is returned.
 * \param __token Token to expect
 */
#define EXPECT_CURRENT_TOKEN(__token)                                       \
do {                                                                        \
	if (m_tokens.get_current_token() != (__token)) {                          \
		return error::emit(                                                     \
			error::code::UNEXPECTED_TOKEN_WITH_EXPECTED,                          \
			m_tokens.get_current_token_location(),                                \
			token(__token).to_string(), m_tokens.get_current_token().to_string()  \
		);                                                                      \
	}                                                                         \
} while(false)

/**
 * \brief Checks, if the nex token is the expected \b __token, if a mismatch
 * is detected, an error is returned.
 * \param __token Token to expect
 */
#define EXPECT_NEXT_TOKEN(__token) \
m_tokens.next();                   \
EXPECT_CURRENT_TOKEN((__token))

namespace sigma {
	parser::parser(frontend_context& context) : m_context(context), m_tokens(context.tokens) {}

	auto parser::parse(frontend_context& context) -> utility::result<void> {
		return parser(context).parse();
	}

	auto parser::parse() -> utility::result<void> {
		// TODO: manage local context (ie. function body, loop body, if body etc), probably use a stack

		while (m_tokens.get_current_token() != token_type::END_OF_FILE) {
			handle<ast::node> result;

			if(m_tokens.get_current_token() == token_type::NAMESPACE) {
				// parse nested namespaces
				TRY(result, parse_namespace_declaration());
			}
			else if(m_tokens.get_current_token() == token_type::STRUCT) {
				// parse global struct declarations
				TRY(result, parse_struct_declaration());
				EXPECT_CURRENT_TOKEN(token_type::SEMICOLON);
				m_tokens.next();
			}
			else if (peek_is_function_definition()) {
				// parse globally declared functions 
				TRY(result, parse_function_declaration());
			}
			else {
				// parse globals
				PANIC("globals aren't implemented yet");
			}

			m_context.syntax.ast.add_node(result);
		}

		return SUCCESS;
	}

	auto parser::parse_namespace_declaration() -> parse_result {
		// expect 'NAMESPACE IDENTIFIER'
		EXPECT_CURRENT_TOKEN(token_type::NAMESPACE);

		// parse the namespace identifier
		EXPECT_NEXT_TOKEN(token_type::IDENTIFIER);
		const utility::string_table_key identifier = m_tokens.get_current().symbol_key;
		const handle<token_location> location = get_current_location();

		// parse contained functions, namespaces, and globals
		m_tokens.next(); // prime the left brace
		TRY(const auto top, parse_namespace_block());

		// create the namespace node
		const handle<ast::node> namespace_node = create_namespace(top.size(), location);
		namespace_node->get<ast::named_expression>().key = identifier;
		utility::copy(namespace_node->children, top);

		return namespace_node;
	}

	auto parser::parse_namespace_block() -> parse_block_result {
		// expect '{ NAMESPACE | FUNCTION | GLOBAL }'
		EXPECT_CURRENT_TOKEN(token_type::LEFT_BRACE);
		std::vector<handle<ast::node>> block;

		m_tokens.next();
		while(m_tokens.get_current_token() != token_type::RIGHT_BRACE) {
			handle<ast::node> parsed = nullptr;

			if(m_tokens.get_current_token() == token_type::NAMESPACE) {
				// parse a nested namespace
				TRY(parsed, parse_namespace_declaration());
			}
			else if (m_tokens.get_current_token() == token_type::STRUCT) {
				// parse global struct declarations
				TRY(parsed, parse_struct_declaration());
				EXPECT_CURRENT_TOKEN(token_type::SEMICOLON);
				m_tokens.next();
			}
			else if(peek_is_function_definition()) {
				// parse a nested function declaration
				TRY(parsed, parse_function_declaration());
			}
			else{
				// parse globals
				PANIC("globals aren't implemented yet");
			}

			block.push_back(parsed);
		}

		EXPECT_CURRENT_TOKEN(token_type::RIGHT_BRACE);
		m_tokens.next(); // prime the next token

		return block;
	}

	auto parser::parse_function_declaration() -> parse_result {
		// expect 'TYPE IDENTIFIER ( TYPE IDENTIFIER, ..., TYPE IDENTIFIER )'
		const handle<token_location> function_location = get_current_location();
		std::vector<named_data_type> parameters;

		// parse the return type
		TRY(const type return_type, parse_type());

		// parse the function identifier
		EXPECT_CURRENT_TOKEN(token_type::IDENTIFIER);
		const utility::string_table_key identifier = m_tokens.get_current().symbol_key;

		// parse the parameter list
		EXPECT_NEXT_TOKEN(token_type::LEFT_PARENTHESIS);
		m_tokens.next();

		if(m_tokens.get_current_token() != token_type::RIGHT_PARENTHESIS) {
			while(true) {
				// parse the parameter type
				TRY(type parameter_type, parse_type());

				// parse the parameter identifier
				EXPECT_CURRENT_TOKEN(token_type::IDENTIFIER);
				auto parameter_identifier = m_tokens.get_current().symbol_key;
				parameters.emplace_back(parameter_type, parameter_identifier);

				// consume the next token and check if it's the end of the parameter list
				m_tokens.next();

				if(m_tokens.get_current_token() == token_type::RIGHT_PARENTHESIS) {
					break; // end of parameter list
				}

				EXPECT_CURRENT_TOKEN(token_type::COMMA);
				m_tokens.next(); // prime the next token for the next iteration
			}
		}

		EXPECT_CURRENT_TOKEN(token_type::RIGHT_PARENTHESIS);

		// parse the function body
		m_tokens.next(); // prime the first block token
		TRY(const std::vector<handle<ast::node>> statements, parse_statement_block());

		utility::block_allocator& allocator = m_context.syntax.ast.get_allocator();

		function_signature signature = {
			.return_type = return_type,
			.parameter_types = utility::slice<named_data_type>(allocator, parameters.size()),
			.identifier_key = identifier
		};

		// create the function node
		const handle<ast::node> function_node = create_function(statements.size(), function_location);

		// initialize the function signature
		auto& function = function_node->get<ast::function>();

		// copy everything over
		function.signature = signature;

		utility::copy(function.signature.parameter_types, parameters);
		utility::copy(function_node->children, statements);

		return function_node;
	}

	auto parser::parse_statement_block() -> parse_block_result {
		// expect '{ STATEMENT, ..., STATEMENT }'
		EXPECT_CURRENT_TOKEN(token_type::LEFT_BRACE);
		m_tokens.next(); // prime the first statement token

		std::vector<handle<ast::node>> statements;

		// parse individual statements 
		while(m_tokens.get_current_token() != token_type::RIGHT_BRACE) {
			TRY(handle<ast::node> statement, parse_statement());
			statements.push_back(statement);
		}

		EXPECT_CURRENT_TOKEN(token_type::RIGHT_BRACE);
		m_tokens.next(); // prime the next token

		return statements;
	}

	auto parser::parse_statement() -> parse_result {
		// expect 'TYPE | IF | RET | IDENTIFIER ... ;'
		handle<ast::node> result;

		if(peek_is_variable_declaration()) {
			TRY(result, parse_variable_declaration());
		}
		else {
			switch (m_tokens.get_current_token()) {
				case token_type::IDENTIFIER: {
					TRY(result, parse_identifier_statement());
					break;
				}
				case token_type::IF: {
					return parse_if_else_statement_block();
				}
				case token_type::RET: {
					TRY(result, parse_return_statement()); break;
				}
				case token_type::STRUCT: {
					TRY(result, parse_struct_declaration()); break;
				}
				default: {
					return error::emit(
						error::code::UNEXPECTED_TOKEN,
						m_tokens.get_current_token_location(),
						m_tokens.get_current_token().to_string()
					);
				}
			}
		}

		// expect a trailing semicolon
		EXPECT_CURRENT_TOKEN(token_type::SEMICOLON);

		m_tokens.next();
		return result;
	}

	auto parser::parse_return_statement() -> parse_result {
		// expect 'RET value'
		EXPECT_CURRENT_TOKEN(token_type::RET);
		const handle<token_location> location = get_current_location();
		m_tokens.next();

		// allow return statements without any expressions
		if(m_tokens.get_current_token() == token_type::SEMICOLON) {
			const handle<ast::node> ret = create_return(0, location);
			return ret;
		}

		// non-semicolon token, parse a return value
		const handle<ast::node> ret = create_return(1, location);
		TRY(ret->children[0], parse_expression());

		return ret;
	}

	auto parser::parse_if_else_statement_block() -> parse_result {
		// expect 'IF ( condition ) { statements } ELSE | ELSE IF ( condition ) { statements }'
		TRY(const handle<ast::node> entry, parse_if_statement());
		handle<ast::node> preceding_branch = entry;

		// parse succeeding if else statements
		while(m_tokens.get_current_token() == token_type::ELSE) {
			if(m_tokens.peek_next_token() == token_type::IF) {
				// if else
				EXPECT_NEXT_TOKEN(token_type::IF);
				TRY(const handle<ast::node> branch, parse_if_statement());

				preceding_branch->children[1] = branch; // point to the next branch
				preceding_branch = branch;
			}
			else if(m_tokens.peek_next_token() == token_type::LEFT_BRACE) {
				// else
				m_tokens.next(); // prime the left brace
				TRY(const std::vector<handle<ast::node>> statements, parse_statement_block());

				// create the final branch node
				const handle<ast::node> branch_node = create_branch(statements.size());
				utility::copy(branch_node->children, statements);

				preceding_branch->children[1] = branch_node; // point to the next branch
				break;
			}
			else {
				const token_info peek = m_tokens.peek_next();
				return error::emit(
					error::code::UNEXPECTED_TOKEN,
					peek.location,
					peek.tok.to_string()
				);
			}
		}

		return entry;
	}

	auto parser::parse_if_statement() -> parse_result {
		// expect 'IF ( condition ) { statements }'
		EXPECT_CURRENT_TOKEN(token_type::IF);
		EXPECT_NEXT_TOKEN(token_type::LEFT_PARENTHESIS);

		// parse the condition
		m_tokens.next(); // prime the expression token
		TRY(const handle<ast::node> condition, parse_expression());

		EXPECT_CURRENT_TOKEN(token_type::RIGHT_PARENTHESIS);

		// parse inner statements 
		m_tokens.next(); // prime the left brace
		TRY(const std::vector<handle<ast::node>> statements, parse_statement_block());

		// create the branch node
		const handle<ast::node> branch_node = create_conditional_branch(statements.size() + 2);

		// copy everything over
		branch_node->children[0] = condition; // condition
		branch_node->children[1] = nullptr;   // false condition target

		// inner statements
		utility::copy(branch_node->children, 2, statements);

		return branch_node;
	}

	auto parser::parse_identifier_statement() -> parse_result {
		// parse a statement which starts with an identifier
		// expect 'NAMESPACES ARRAY_ACCESS | MEMBER_ACCESS'

		// parse a namespace, if there is one
		TRY(const namespace_list namespaces, parse_namespaces());

		if (peek_is_function_call()) {
			return parse_function_call(namespaces);
		}

		TRY(handle<ast::node> result, parse_variable_access());

		while(true) {
			// array access
			if(m_tokens.get_current_token() == token_type::LEFT_BRACKET) {
				// [indices]
				TRY(handle<ast::node> array_access, parse_array_access());
				array_access->children[0] = result;
				result = array_access;
			}
			else if(m_tokens.get_current_token() == token_type::DOT) {
				// .member
				TRY(handle<ast::node> member_access, parse_local_member_access());
				member_access->children[0] = result;
				result = member_access;
			}
			else {
				break;
			}
		}

		if(m_tokens.get_current_token() == token_type::EQUALS_SIGN) {
			TRY(const handle<ast::node> assignment_node, parse_assignment());

			assignment_node->children[0] = result;
			return assignment_node;
		}

		return result;
	
		// // check if we're assigning anything
		// // TODO: replace by a store op
		// if (m_tokens.get_current_token() == token_type::EQUALS_SIGN) {
		// 	TRY(const handle<ast::node> assignment_node, parse_assignment());
		// 
		// 	if(array_access) {
		// 		// assignment to an array access
		// 		array_access->children[0] = variable;
		// 		assignment_node->children[0] = array_access;
		// 	}
		// 	else {
		// 		assignment_node->children[0] = variable;
		// 	}
		// 
		// 	return assignment_node;
		// }
		// 
		// if(array_access) {
		// 	// plain array access
		// 	array_access->children[0] = variable;
		// 	return array_access;
		// }

		// return variable;
	}

	auto parser::parse_local_member_access() -> parse_result {
		// expect '.IDENTIFIER'
		EXPECT_CURRENT_TOKEN(token_type::DOT);
		const handle<token_location> location = get_current_location();

		EXPECT_NEXT_TOKEN(token_type::IDENTIFIER);
		const utility::string_table_key identifier = m_tokens.get_current().symbol_key;

		const handle<ast::node> access_node = create_local_member_access(location);
		access_node->get<ast::named_type_expression>().key = identifier;

		m_tokens.next();
		return access_node;
	}

	auto parser::parse_negative_expression() -> parse_result {
		EXPECT_CURRENT_TOKEN(token_type::MINUS_SIGN);
		const handle<token_location> location = get_current_location();
		m_tokens.next(); // prime the first expression token

		TRY(const handle<ast::node> expression_node, parse_expression());
		const handle<ast::node> negation_node = create_numerical_literal(location);

		ast::named_type_expression& literal = negation_node->get<ast::named_type_expression>();
		literal.key = m_context.syntax.strings.insert("-1");
		literal.type = type::create_i32();

		// negate the expression
		return create_binary_operation(ast::node_type::OPERATOR_MULTIPLY, negation_node, expression_node);
	}

	auto parser::parse_function_call(const namespace_list& namespaces) -> parse_result {
		// expect 'IDENTIFIER ( PARAMETER, ... , PARAMETER )'
		EXPECT_CURRENT_TOKEN(token_type::IDENTIFIER);

		const handle<token_location> call_location = get_current_location();
		const utility::string_table_key identifier_key = m_tokens.get_current().symbol_key;
		std::vector<handle<ast::node>> parameters;

		EXPECT_NEXT_TOKEN(token_type::LEFT_PARENTHESIS);
		m_tokens.next();

		// parse call parameters
		if(m_tokens.get_current_token() != token_type::RIGHT_PARENTHESIS) {
			while(true) {
				TRY(handle<ast::node> expression_node, parse_expression());
				parameters.push_back(expression_node);

				if(m_tokens.get_current_token() == token_type::RIGHT_PARENTHESIS) {
					break; // end of parameter list
				}

				EXPECT_CURRENT_TOKEN(token_type::COMMA);
				m_tokens.next();
			}
		}

		EXPECT_CURRENT_TOKEN(token_type::RIGHT_PARENTHESIS);

		// create the callee
		const handle<ast::node> call_node = create_function_call(parameters.size(), call_location);

		// copy over function parameters
		utility::copy(call_node->children, parameters);

		// initialize the callee
		ast::function_call& function = call_node->get<ast::function_call>();
		function.signature.identifier_key = identifier_key;
		function.namespaces = namespaces;

		m_tokens.next();
		return call_node;
	}

	auto parser::parse_variable_declaration() -> parse_result {
		// expect 'TYPE IDENTIFIER'
		const handle<token_location> location = get_current_location();
		handle<ast::node> assigned_value = nullptr;

		// parse the variable type
		TRY(const type type, parse_type());

		// parse the identifier
		EXPECT_CURRENT_TOKEN(token_type::IDENTIFIER);
		const auto identifier_key = m_tokens.get_current().symbol_key;
		m_tokens.next();

		// parse an assignment, if there is one
		if(m_tokens.get_current_token() == token_type::EQUALS_SIGN) {
			m_tokens.next();
			TRY(assigned_value, parse_expression());
		}

		// create the variable node
		handle<ast::node> variable_node;

		if(assigned_value) {
			variable_node = create_variable_declaration(1, location);
			variable_node->children[0] = assigned_value;
		}
		else {
			variable_node = create_variable_declaration(0, location);
		}

		// initialize the variable
		auto& variable = variable_node->get<ast::named_type_expression>();
		variable.key = identifier_key;
		variable.type = type;

		return variable_node;
	}

	auto parser::parse_variable_access() -> parse_result {
		// expect 'IDENTIFIER'
		EXPECT_CURRENT_TOKEN(token_type::IDENTIFIER);
		const handle<token_location> location = get_current_location();

		// create the access node
		const handle<ast::node> variable_node = create_variable_access(location);

		// initialize the variable
		auto& variable = variable_node->get<ast::named_type_expression>();
		variable.key = m_tokens.get_current().symbol_key;

		m_tokens.next();
		return variable_node;
	}

	auto parser::parse_array_access() -> parse_result {
		// parses an array index access
		// expect '[index]'
		EXPECT_CURRENT_TOKEN(token_type::LEFT_BRACKET);
		const handle<token_location> location = get_current_location();

		m_tokens.next(); // prime the first expression token
		TRY(const handle<ast::node> index, parse_expression());

		EXPECT_CURRENT_TOKEN(token_type::RIGHT_BRACKET);

		// create the access node
		const handle<ast::node> array_access = create_array_access(2, location);
		array_access->children[1] = index;

		m_tokens.next(); // prime the next token
		return array_access;
	}

	auto parser::parse_sizeof() -> parse_result {
		// expect 'SIZEOF ( type )'
		EXPECT_CURRENT_TOKEN(token_type::SIZEOF);
		const handle<token_location> location = get_current_location();

		EXPECT_NEXT_TOKEN(token_type::LEFT_PARENTHESIS);

		// parse the type
		m_tokens.next(); // prime the type token
		TRY(const type type, parse_type());

		EXPECT_CURRENT_TOKEN(token_type::RIGHT_PARENTHESIS);

		// create the sizeof node
		const handle<ast::node> sizeof_node = create_sizeof(location);
		sizeof_node->get<ast::type_expression>().type = type;

		m_tokens.next();
		return sizeof_node;
	}

	auto parser::parse_assignment() -> parse_result {
		// expect '= expression'
		EXPECT_CURRENT_TOKEN(token_type::EQUALS_SIGN);

		const handle<token_location> location = get_current_location();
		m_tokens.next(); 

		// create the assignment node
		const handle<ast::node> assignment_node = create_assignment(location);

		// handle the assigned expression
		TRY(assignment_node->children[1], parse_expression()); 

		return assignment_node;
	}

	auto parser::parse_logical_not_expression() -> parse_result {
		// expect '! IDENTIFIER_STATEMENT'
		EXPECT_CURRENT_TOKEN(token_type::EXCLAMATION_MARK);
		const handle<token_location> location = get_current_location();

		m_tokens.next(); // prime the expression token

		if(m_tokens.get_current_token() == token_type::EXCLAMATION_MARK) {
			// don't allow negations of negations and so on
			return error::emit(error::code::MULTIPLE_NEGATIONS, location);
		}

		TRY(const handle<ast::node> expression, parse_primary());

		// create the not node
		const handle<ast::node> not_expression = create_logical_not(location);
		not_expression->children[0] = expression;

		return not_expression;
	}

	auto parser::parse_expression() -> parse_result {
		return parse_logical_conjunction();
	}

	auto parser::parse_logical_conjunction() -> parse_result {
		TRY(handle<ast::node> left_node, parse_logical_disjunction());

		while(m_tokens.get_current_token() == token_type::CONJUNCTION) {
			m_tokens.next(); // prime the term
			TRY(const handle<ast::node> right_node, parse_logical_disjunction());

			left_node = create_binary_operation(ast::node_type::OPERATOR_CONJUNCTION, left_node, right_node);
		}

		return left_node;
	}

	auto parser::parse_logical_disjunction() -> parse_result {
		TRY(handle<ast::node> left_node, parse_comparison());

		while(m_tokens.get_current_token() == token_type::DISJUNCTION) {
			m_tokens.next(); // prime the term
			TRY(const handle<ast::node> right_node, parse_comparison());

			left_node = create_binary_operation(ast::node_type::OPERATOR_DISJUNCTION, left_node, right_node);
		}

		return left_node;
	}

	auto parser::parse_comparison() -> parse_result {
		TRY(handle<ast::node> left_node, parse_term());
		token operation = m_tokens.get_current_token();

		while (
			operation == token_type::GREATER_THAN_OR_EQUAL ||
			operation == token_type::LESS_THAN_OR_EQUAL ||
			operation == token_type::GREATER_THAN ||
			operation == token_type::NOT_EQUALS ||
			operation == token_type::LESS_THAN ||
			operation == token_type::EQUALS
		) {
			m_tokens.next(); // prime the comparison

			TRY(const handle<ast::node> right_node, parse_term());
			ast::node_type operator_type = ast::node_type::UNKNOWN;

			switch (operation) {
				case token_type::GREATER_THAN_OR_EQUAL:  operator_type = ast::node_type::OPERATOR_GREATER_THAN_OR_EQUAL; break;
				case token_type::LESS_THAN_OR_EQUAL:     operator_type = ast::node_type::OPERATOR_LESS_THAN_OR_EQUAL; break;
				case token_type::GREATER_THAN:           operator_type = ast::node_type::OPERATOR_GREATER_THAN; break;
				case token_type::NOT_EQUALS:             operator_type = ast::node_type::OPERATOR_NOT_EQUAL; break;
				case token_type::LESS_THAN:              operator_type = ast::node_type::OPERATOR_LESS_THAN; break;
				case token_type::EQUALS:                 operator_type = ast::node_type::OPERATOR_EQUAL; break;
				default: PANIC("unhandled term case for token '{}'", operation.to_string());
			}

			left_node = create_comparison_operation(operator_type, left_node, right_node);
			operation = m_tokens.get_current_token();
		}

		return left_node;
	}

	auto parser::parse_term() -> parse_result {
		TRY(handle<ast::node> left_node, parse_factor());
		token operation = m_tokens.get_current_token();

		while (
			operation == token_type::PLUS_SIGN ||
			operation == token_type::MINUS_SIGN
		) {
			m_tokens.next(); // prime the term

			TRY(const handle<ast::node> right_node, parse_factor());
			ast::node_type operator_type = ast::node_type::UNKNOWN;

			switch (operation) {
				case token_type::PLUS_SIGN:  operator_type = ast::node_type::OPERATOR_ADD; break;
				case token_type::MINUS_SIGN: operator_type = ast::node_type::OPERATOR_SUBTRACT; break;
				default: PANIC("unhandled term case for token '{}'", operation.to_string());
			}

			left_node = create_binary_operation(operator_type, left_node, right_node);
			operation = m_tokens.get_current_token();
		}

		return left_node;
	}

	auto parser::parse_factor() -> parse_result {
		TRY(handle<ast::node> left_node, parse_primary());
		token operation = m_tokens.get_current_token();

		while (
			operation == token_type::ASTERISK ||
			operation == token_type::SLASH ||
			operation == token_type::MODULO
		) {
			m_tokens.next(); // prime the primary

			TRY(const handle<ast::node> right_node, parse_primary());
			ast::node_type operator_type = ast::node_type::UNKNOWN;

			switch (operation) {
				case token_type::ASTERISK: operator_type = ast::node_type::OPERATOR_MULTIPLY; break;
				case token_type::SLASH:    operator_type = ast::node_type::OPERATOR_DIVIDE; break;
				case token_type::MODULO:   operator_type = ast::node_type::OPERATOR_MODULO; break;
				default: PANIC("unhandled factor case for token '{}'", operation.to_string());
			}

			left_node = create_binary_operation(operator_type, left_node, right_node);
			operation = m_tokens.get_current_token();
		}

		return left_node;
	}

	auto parser::parse_primary() -> parse_result {
		// parse a numerical literals
		if(m_tokens.get_current_token().is_numerical_literal()) {
			return parse_numerical_literal();
		}

		// parse an explicit cast
		if(peek_is_explicit_cast()) {
			return parse_explicit_cast();
		}

		// fallback parsers
		switch (m_tokens.get_current_token()) {
			case token_type::IDENTIFIER:         return parse_identifier_expression();
			case token_type::ALIGNOF:            return parse_alignof();
			case token_type::SIZEOF:             return parse_sizeof();

			// modifiers
			case token_type::MINUS_SIGN:         return parse_negative_expression();
			case token_type::EXCLAMATION_MARK:   return parse_logical_not_expression();

			// literals
			case token_type::STRING_LITERAL:     return parse_string_literal();
			case token_type::CHARACTER_LITERAL:  return parse_character_literal();
			case token_type::BOOL_LITERAL_TRUE:
			case token_type::BOOL_LITERAL_FALSE: return parse_bool_literal();
			case token_type::NULL_LITERAL:       return parse_null_literal();
			default: {
				return error::emit(
					error::code::UNEXPECTED_TOKEN, 
					m_tokens.get_current_token_location(), 
					m_tokens.get_current_token().to_string()
				);
			}
		}
	}

	auto parser::parse_explicit_cast() -> parse_result {
		// expect 'CAST < TYPE >'
		EXPECT_CURRENT_TOKEN(token_type::CAST);
		const handle<token_location> location = get_current_location();

		EXPECT_NEXT_TOKEN(token_type::LESS_THAN);

		// parse the target type
		m_tokens.next(); // prime the type token
		TRY(const type target_type, parse_type());

		EXPECT_CURRENT_TOKEN(token_type::GREATER_THAN);
		EXPECT_NEXT_TOKEN(token_type::LEFT_PARENTHESIS);

		// parse the expression we want to cast
		m_tokens.next(); // prime the statement
		TRY(const handle<ast::node> value_to_cast, parse_expression());

		EXPECT_CURRENT_TOKEN(token_type::RIGHT_PARENTHESIS);
		m_tokens.next(); // prime the next token (probably a SEMICOLON)

		// create the cast node
		const handle<ast::node> cast_node = create_cast(location);
		cast_node->children[0] = value_to_cast;

		ast::cast& cast = cast_node->get<ast::cast>();
		cast.target_type = target_type;

		return cast_node;
	}

	auto parser::parse_type() -> utility::result<type> {
		// expect '(TYPE | IDENTIFIER)* ... *'
		TRY(const namespace_list namespaces, parse_namespaces());

		const token_info type_token = m_tokens.get_current();
		u8 pointer_level = 0;

		// parse the base type
		if(!is_current_token_type()) {
			return error::emit(
				error::code::UNEXPECTED_TOKEN,
				m_tokens.get_current_token_location(),
				type_token.tok.to_string()
			);
		}

		// parse pointers
		while(m_tokens.peek_next_token() == token_type::ASTERISK) {
			m_tokens.next();
			pointer_level++;
		}

		m_tokens.next();

		type ty(type_token, pointer_level);
		ty.set_namespaces(namespaces);
		return ty;
	}

	auto parser::parse_numerical_literal() -> parse_result {
		// expect 'NUMERICAL_LITERAL'
		const token literal_token = m_tokens.get_current_token();
		const handle<token_location> location = get_current_location();

		if(!literal_token.is_numerical_literal()) {
			return error::emit(
				error::code::UNEXPECTED_NON_NUMERICAL,
				m_tokens.get_current_token_location(),
				literal_token.to_string()
			);
		}

		type::kind base = type::UNKNOWN;

		// these types won't be used most of the time, but its nice to hav a place we can
		// fall back to
		switch (literal_token) {
			case token_type::UNSIGNED_LITERAL:    base = type::U32; break;
			//case lex::token_type::F32_LITERAL:         base = data_type::F32; break;
			//case lex::token_type::F64_LITERAL:         base = data_type::F64; break;
			case token_type::SIGNED_LITERAL:
			case token_type::HEXADECIMAL_LITERAL:
			case token_type::BINARY_LITERAL:      base = type::I32; break;
			default: PANIC("unhandled literal to data type conversion '{}'", literal_token.to_string());
		}

		// create the literal node
		const handle<ast::node> literal_node = create_numerical_literal(location);

		// initialize the literal
		auto& literal = literal_node->get<ast::named_type_expression>();
		literal.key = m_tokens.get_current().symbol_key;
		literal.type = { base, 0 };

		m_tokens.next();
		return literal_node;
	}

	auto parser::parse_character_literal() -> parse_result {
		// expect 'CHARACTER_LITERAL'
		EXPECT_CURRENT_TOKEN(token_type::CHARACTER_LITERAL);
		const handle<token_location> location = get_current_location();

		// create the string node
		const handle<ast::node> char_node = create_character_literal(location);

		// initialize the literal
		auto& literal = char_node->get<ast::named_type_expression>();
		literal.key = m_tokens.get_current().symbol_key;
		literal.type = type::create_char(); // char

		m_tokens.next();
		return char_node;
	}

	auto parser::parse_string_literal() -> parse_result {
		// expect 'STRING_LITERAL'
		EXPECT_CURRENT_TOKEN(token_type::STRING_LITERAL);
		const handle<token_location> location = get_current_location();

		// create the string node
		const handle<ast::node> string_node = create_string_literal(location);

		// initialize the literal
		auto& literal = string_node->get<ast::named_type_expression>();
		literal.key = m_tokens.get_current().symbol_key;
		literal.type = type::create_char(1); // char*

		m_tokens.next();
		return string_node;
	}

	auto parser::parse_bool_literal() -> parse_result {
		// expect 'BOOL_LITERAL_TRUE | BOOL_LITERAL_FALSE'
		const handle<token_location> location = get_current_location();

		ASSERT(
			m_tokens.get_current_token() == token_type::BOOL_LITERAL_FALSE ||
			m_tokens.get_current_token() == token_type::BOOL_LITERAL_TRUE, 
			"invalid token - expected an a boolean literal"
		);

		// create the bool node
		const handle<ast::node> bool_node = create_bool_literal(location);

		// initialize the literal
		const bool value = m_tokens.get_current_token() == token_type::BOOL_LITERAL_TRUE;
		bool_node->get<ast::bool_literal>().value = value;

		m_tokens.next();
		return bool_node;
	}

	auto parser::parse_null_literal() -> parse_result {
		// expect 'NULL_LITERAL'
		const handle<token_location> location = get_current_location();
		EXPECT_CURRENT_TOKEN(token_type::NULL_LITERAL);

		// create the null literal node
		const handle<ast::node> null_node = create_null_literal(location);

		m_tokens.next();
		return null_node;
	}

	auto parser::is_current_token_type() const -> bool {
		// TODO: handle generics
		// TODO: handle pointers
		// TODO: handle static arrays

		// starting token is presumably the beginning token of a type
		const token token = m_tokens.get_current_token();

		// interpret identifiers as types, this will be handled later in the type checker
		return token == token_type::IDENTIFIER || token.is_type();
	}

	auto parser::get_current_location() const -> handle<token_location> {
		return m_tokens.get_current_token_location();
	}

	auto parser::parse_namespaces() -> utility::result<namespace_list> {
		// parses a set of contiguous namespace directives
		// expect 'NAMESPACE :: ... NAMESPACE ::'
		std::vector<utility::string_table_key> namespaces;

		while(peek_is_namespace_access()) {
			// expect 'IDENTIFIER ::'
			EXPECT_CURRENT_TOKEN(token_type::IDENTIFIER);
			namespaces.push_back(m_tokens.get_current().symbol_key);

			// sanity checks 
			EXPECT_NEXT_TOKEN(token_type::COLON);
			EXPECT_NEXT_TOKEN(token_type::COLON);

			m_tokens.next(); // prime the next token
		}

		utility::slice<utility::string_table_key> list(m_context.allocator, namespaces.size());
		utility::copy(list, namespaces);

		return { list };
	}

	auto parser::peek_is_function_definition() -> bool {
		// peek 'TYPE IDENTIFIER ('
		// starting token is presumably the beginning token of a type
		if (!is_current_token_type()) {
			return false;
		}

		if (m_tokens.peek_token() != token_type::IDENTIFIER) {
			m_tokens.synchronize_indices();
			return false;
		}

		const bool res = m_tokens.peek_token() == token_type::LEFT_PARENTHESIS;
		m_tokens.synchronize_indices();
		return res;
	}

	auto parser::peek_is_explicit_cast() const -> bool {
		// peek 'CAST <'
		if(m_tokens.get_current_token() != token_type::CAST) {
			return false;
		}

		return m_tokens.peek_next_token() == token_type::LESS_THAN;
	}

	auto parser::peek_is_function_call() const -> bool {
		// peek 'IDENTIFIER ('
		if (m_tokens.get_current_token() != token_type::IDENTIFIER) {
			return false;
		}

		// TODO: this will have to handle generics when time comes
		return m_tokens.peek_next_token() == token_type::LEFT_PARENTHESIS;
	}

	auto parser::parse_alignof() -> parse_result {
		// expect 'ALIGNOF ( type )'
		EXPECT_CURRENT_TOKEN(token_type::ALIGNOF);
		const handle<token_location> location = get_current_location();

		EXPECT_NEXT_TOKEN(token_type::LEFT_PARENTHESIS);

		// parse the type
		m_tokens.next(); // prime the type token
		TRY(const type type, parse_type());

		EXPECT_CURRENT_TOKEN(token_type::RIGHT_PARENTHESIS);

		// create the alignof node
		const handle<ast::node> sizeof_node = create_alignof(location);
		sizeof_node->get<ast::type_expression>().type = type;

		m_tokens.next();
		return sizeof_node;
	}

	auto parser::peek_is_variable_declaration() -> bool {
		// peek 'NAMESPACES TYPE * ... * IDENTIFIER'
		if(m_tokens.get_current_token() == token_type::IDENTIFIER) {
			u64 depth = 0;
			while(
				m_tokens.peek_token() == token_type::COLON &&
				m_tokens.peek_token() == token_type::COLON &&
				m_tokens.peek_token() == token_type::IDENTIFIER
			) { depth++; }

			m_tokens.synchronize_indices();

			// consume all the namespaces
			for(u64 i = 0; i < depth; ++i) {
				m_tokens.peek_token(); // IDENTIFIER
				m_tokens.peek_token(); // COLON
				m_tokens.peek_token(); // COLON
			}
		}

		if (!is_current_token_type()) {
			m_tokens.synchronize_indices();
			return false;
		}

		// consume pointers
		while(m_tokens.peek_token() == token_type::ASTERISK) {}

		const bool res = m_tokens.get_current_peek_token() == token_type::IDENTIFIER;
		m_tokens.synchronize_indices();
		return res;
	}

	auto parser::peek_is_namespace_access() -> bool {
		// peek 'IDENTIFIER::'
		if(m_tokens.get_current_token() != token_type::IDENTIFIER) {
			return false;
		}

		if (m_tokens.peek_token() != token_type::COLON) {
			m_tokens.synchronize_indices();
			return false;
		}

		const bool res = m_tokens.peek_token() == token_type::COLON;
		m_tokens.synchronize_indices();
		return res;
	}

	auto parser::create_local_member_access(handle<token_location> location) const -> handle<ast::node> {
		return create_node<ast::named_type_expression>(ast::node_type::LOCAL_MEMBER_ACCESS, 1, location);
	}

	auto parser::create_numerical_literal(handle<token_location> location) const -> handle<ast::node> {
		return create_node<ast::named_type_expression>(ast::node_type::NUMERICAL_LITERAL, 0, location);
	}

	auto parser::create_character_literal(handle<token_location> location) const -> handle<ast::node> {
		return create_node<ast::named_type_expression>(ast::node_type::CHARACTER_LITERAL, 0, location);
	}

	auto parser::create_variable_access(handle<token_location> location) const -> handle<ast::node> {
		return create_node<ast::named_type_expression>(ast::node_type::VARIABLE_ACCESS, 0, location);
	}

	auto parser::create_string_literal(handle<token_location> location) const -> handle<ast::node> {
		return create_node<ast::named_type_expression>(ast::node_type::STRING_LITERAL, 0, location);
	}

	auto parser::create_bool_literal(handle<token_location> location) const -> handle<ast::node> {
		return create_node<ast::bool_literal>(ast::node_type::BOOL_LITERAL, 0, location);
	}

	auto parser::create_null_literal(handle<token_location> location) const -> handle<ast::node> {
		return create_node<utility::empty_property>(ast::node_type::NULL_LITERAL, 0, location);
	}

	auto parser::create_assignment(handle<token_location> location) const -> handle<ast::node> {
		return create_node<utility::empty_property>(ast::node_type::STORE, 2, location);
	}

	auto parser::create_sizeof(handle<token_location> location) const -> handle<ast::node> {
		return create_node<ast::type_expression>(ast::node_type::SIZEOF, 0, location);
	}

	auto parser::create_alignof(handle<token_location> location) const -> handle<ast::node> {
		return create_node<ast::type_expression>(ast::node_type::ALIGNOF, 0, location);
	}

	auto parser::create_cast(handle<token_location> location) const -> handle<ast::node> {
		return create_node<ast::cast>(ast::node_type::CAST, 1, location);
	}

	auto parser::create_logical_not(handle<token_location> location) const -> handle<ast::node> {
		return create_node(ast::node_type::OPERATOR_LOGICAL_NOT, 1, location);
	}

  auto parser::create_comparison_operation(ast::node_type type, handle<ast::node> left, handle<ast::node> right) const -> handle<ast::node> {
		const handle<ast::node> node = create_node<ast::comparison_expression>(type, 2, left->location);
		node->children[0] = left;
		node->children[1] = right;

		return node;
  }

	auto parser::create_variable_declaration(u64 child_count, handle<token_location> location) const -> handle<ast::node> {
		return create_node<ast::named_type_expression>(ast::node_type::VARIABLE_DECLARATION, child_count, location);
	}

	auto parser::create_function_call(u64 child_count, handle<token_location> location) const -> handle<ast::node> {
		return create_node<ast::function_call>(ast::node_type::FUNCTION_CALL, child_count, location);
	}

	auto parser::create_array_access(u64 child_count, handle<token_location> location) const -> handle<ast::node> {
		return create_node<ast::type_expression>(ast::node_type::ARRAY_ACCESS, child_count, location);
	}

	auto parser::create_namespace(u64 child_count, handle<token_location> location) const -> handle<ast::node> {
		return create_node<ast::named_expression>(ast::node_type::NAMESPACE_DECLARATION, child_count, location);
	}

	auto parser::create_function(u64 child_count, handle<token_location> location) const -> handle<ast::node> {
		return create_node<ast::function>(ast::node_type::FUNCTION_DECLARATION, child_count, location);
	}

	auto parser::create_return(u64 child_count, handle<token_location> location) const -> handle<ast::node> {
		return create_node(ast::node_type::RETURN, child_count, location);
	}

	auto parser::create_branch(u64 child_count) const -> handle<ast::node> {
		return create_node(ast::node_type::BRANCH, child_count, nullptr);
	}

	auto parser::create_struct_declaration(handle<token_location> location) const -> handle<ast::node> {
		return create_node<ast::named_type_expression>(ast::node_type::STRUCT_DECLARATION, 0, location);
	}

	auto parser::create_conditional_branch(u64 child_count) const -> handle<ast::node> {
		return create_node(ast::node_type::CONDITIONAL_BRANCH, child_count, nullptr);
	}

	auto parser::create_binary_operation(ast::node_type type, handle<ast::node> left, handle<ast::node> right) const -> handle<ast::node> {
		const handle<ast::node> node = create_node(type, 2, left->location);
		node->children[0] = left;
		node->children[1] = right;

		return node;
	}

	auto parser::parse_identifier_expression() -> parse_result {
		// parse a statement which starts with an identifier
		// expect 'NAMESPACES ARRAY_ACCESS | MEMBER_ACCESS'

		// parse a namespace, if there is one
		TRY(const namespace_list namespaces, parse_namespaces());

		if (peek_is_function_call()) {
			return parse_function_call(namespaces);
		}

		const handle<token_location> location = get_current_location();
		const handle<ast::node> load_node = create_node<ast::type_expression>(ast::node_type::LOAD, 1, location);

		// parse a variable value
		TRY(handle<ast::node> result, parse_variable_access());

		while (true) {
			// array access
			if (m_tokens.get_current_token() == token_type::LEFT_BRACKET) {
				// [indices]
				TRY(handle<ast::node> array_access, parse_array_access());
				array_access->children[0] = result;
				result = array_access;
			}
			else if (m_tokens.get_current_token() == token_type::DOT) {
				// .member
				TRY(handle<ast::node> member_access, parse_local_member_access());
				member_access->children[0] = result;
				result = member_access;
			}
			else {
				break;
			}
		}

		load_node->children[0] = result;
		return load_node;
	}

	auto parser::parse_struct_declaration() -> parse_result {
		// expect 'STRUCT IDENTIFIER { TYPE IDENTIFIER ... TYPE IDENTIFIER }
		const handle<token_location> location = get_current_location();

		// parse the struct header
		EXPECT_CURRENT_TOKEN(token_type::STRUCT);
		EXPECT_NEXT_TOKEN(token_type::IDENTIFIER);
		const utility::string_table_key identifier = m_tokens.get_current().symbol_key;

		// parse inner types 
		EXPECT_NEXT_TOKEN(token_type::LEFT_BRACE);
		m_tokens.next(); // prime the first type token

		std::vector<type> members;

		while(m_tokens.get_current_token() != token_type::RIGHT_BRACE) {
			TRY(type type, parse_type());
			EXPECT_CURRENT_TOKEN(token_type::IDENTIFIER);

			// parse the type identifier
			const utility::string_table_key type_identifier = m_tokens.get_current().symbol_key;
			EXPECT_NEXT_TOKEN(token_type::SEMICOLON);
			m_tokens.next();

			members.emplace_back(type::create_member(type, type_identifier));
		}

		EXPECT_CURRENT_TOKEN(token_type::RIGHT_BRACE);
		m_tokens.next(); // prime the next token

		const handle<ast::node> struct_node = create_struct_declaration(location);

		ASSERT(members.size() < std::numeric_limits<u8>::max(), "too many members");
		utility::slice<type, u8> member_slice(m_context.allocator, static_cast<u8>(members.size()));
		utility::copy(member_slice, members);

		auto& expression = struct_node->get<ast::named_type_expression>();
		expression.type = type::create_struct(member_slice, identifier);
		expression.key = identifier;

		return struct_node;
	}
} // namespace sigma
