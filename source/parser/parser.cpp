#include "parser.h"
#include <compiler/compiler/compilation_context.h>
#include <compiler/compiler/diagnostics.h>

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
			handle<node> result;

			if(m_tokens.get_current_token() == token_type::NAMESPACE) {
				// parse nested namespaces
				TRY(result, parse_namespace_declaration());
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

	auto parser::parse_namespace_declaration() -> utility::result<handle<node>> {
		// expect 'NAMESPACE IDENTIFIER'
		EXPECT_CURRENT_TOKEN(token_type::NAMESPACE);

		// parse the namespace identifier
		EXPECT_NEXT_TOKEN(token_type::IDENTIFIER);
		const utility::string_table_key identifier_key = m_tokens.get_current().symbol_key;
		const handle<token_location> location = m_tokens.get_current_token_location();

		// parse contained functions, namespaces, and globals
		m_tokens.next(); // prime the left brace
		TRY(const auto top, parse_namespace_block());

		const handle<node> namespace_node = create_node<ast_namespace>(
			node_type::NAMESPACE_DECLARATION, top.size(), location
		);

		std::memcpy(namespace_node->children.get_data(), top.data(), top.size() * sizeof(handle<node>));
		namespace_node->get<ast_namespace>().identifier_key = identifier_key;

		return namespace_node;
	}

	auto parser::parse_namespace_block() -> utility::result<std::vector<handle<node>>> {
		// expect '{ NAMESPACE | FUNCTION | GLOBAL }'
		EXPECT_CURRENT_TOKEN(token_type::LEFT_BRACE);
		std::vector<handle<node>> block;

		m_tokens.next();
		while(m_tokens.get_current_token() != token_type::RIGHT_BRACE) {
			handle<node> parsed = nullptr;

			if(m_tokens.get_current_token() == token_type::NAMESPACE) {
				// parse a nested namespace
				TRY(parsed, parse_namespace_declaration());
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

	auto parser::parse_function_declaration() -> utility::result<handle<node>> {
		// expect 'TYPE IDENTIFIER ( TYPE IDENTIFIER, ..., TYPE IDENTIFIER )'
		const handle<token_location> function_location = m_tokens.get_current_token_location();
		std::vector<named_data_type> parameters;

		// parse the return type
		TRY(const data_type return_type, parse_type());

		// parse the function identifier
		EXPECT_CURRENT_TOKEN(token_type::IDENTIFIER);
		const utility::string_table_key identifier_key = m_tokens.get_current().symbol_key;

		// parse the parameter list
		EXPECT_NEXT_TOKEN(token_type::LEFT_PARENTHESIS);
		m_tokens.next();

		if(m_tokens.get_current_token() != token_type::RIGHT_PARENTHESIS) {
			while(true) {
				// parse the parameter type
				TRY(data_type parameter_type, parse_type());

				// parse the parameter identifier
				EXPECT_CURRENT_TOKEN(token_type::IDENTIFIER);
				auto parameter_identifier_key = m_tokens.get_current().symbol_key;
				parameters.emplace_back(parameter_type, parameter_identifier_key);

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
		TRY(const std::vector<handle<node>> statements, parse_statement_block());

		utility::block_allocator& allocator = m_context.syntax.ast.get_allocator();

		function_signature signature = {
			.return_type = return_type,
			.parameter_types = utility::slice<named_data_type>(allocator, parameters.size()),
			.identifier_key = identifier_key
		};

		const handle<node> function_node = create_node<ast_function>(
			node_type::FUNCTION_DECLARATION, statements.size(), function_location
		);

		// initialize the function signature
		auto& function = function_node->get<ast_function>();
		function.signature = signature;

		// copy all statement pointers over to the memory arena
		std::memcpy(function_node->children.get_data(), statements.data(), statements.size() * sizeof(handle<node>));
		std::memcpy(function.signature.parameter_types.get_data(), parameters.data(), parameters.size() * sizeof(named_data_type));

		return function_node;
	}

	auto parser::parse_statement_block() -> utility::result<std::vector<handle<node>>> {
		// expect '{ STATEMENT, ..., STATEMENT }'
		EXPECT_CURRENT_TOKEN(token_type::LEFT_BRACE);
		m_tokens.next(); // prime the first statement token

		std::vector<handle<node>> statements;

		// parse individual statements 
		while(m_tokens.get_current_token() != token_type::RIGHT_BRACE) {
			TRY(handle<node> statement, parse_statement());
			statements.push_back(statement);
		}

		EXPECT_CURRENT_TOKEN(token_type::RIGHT_BRACE);
		m_tokens.next(); // prime the next token

		return statements;
	}

	auto parser::parse_statement() -> utility::result<handle<node>> {
		// expect 'TYPE | IF | RET | IDENTIFIER ... ;'
		handle<node> result;

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
				default: {
					return error::emit(error::code::UNEXPECTED_TOKEN, m_tokens.get_current_token_location(), m_tokens.get_current_token().to_string());
				}
			}
		}

		// expect a trailing semicolon
		EXPECT_CURRENT_TOKEN(token_type::SEMICOLON);

		m_tokens.next();
		return result;
	}

	auto parser::parse_return_statement() -> utility::result<handle<node>> {
		// expect 'RET value'
		EXPECT_CURRENT_TOKEN(token_type::RET);
		const handle<token_location> location = m_tokens.get_current_token_location();
		m_tokens.next();

		// allow return statements without any expressions
		if(m_tokens.get_current_token() == token_type::SEMICOLON) {
			const handle<node> ret = create_node<utility::empty_property>(node_type::RETURN, 0, location);
			return ret;
		}

		// non-semicolon token, parse a return value
		const handle<node> ret = create_node<utility::empty_property>(node_type::RETURN, 1, location);
		TRY(ret->children[0], parse_expression());

		return ret;
	}

	auto parser::parse_if_else_statement_block() -> utility::result<handle<node>> {
		// expect 'IF ( condition ) { statements } ELSE | ELSE IF ( condition ) { statements }'
		TRY(const handle<node> entry, parse_if_statement());
		handle<node> preceding_branch = entry;

		// parse succeeding if else statements
		while(m_tokens.get_current_token() == token_type::ELSE) {
			if(m_tokens.peek_next_token() == token_type::IF) {
				// if else
				EXPECT_NEXT_TOKEN(token_type::IF);
				TRY(const handle<node> branch, parse_if_statement());

				preceding_branch->children[1] = branch; // point to the next branch
				preceding_branch = branch;
			}
			else if(m_tokens.peek_next_token() == token_type::LEFT_BRACE) {
				// else
				m_tokens.next(); // prime the left brace
				TRY(const std::vector<handle<node>> statements, parse_statement_block());

				const handle<node> branch_node = create_node<utility::empty_property>(node_type::BRANCH, statements.size(), nullptr);
				std::memcpy(branch_node->children.get_data(), statements.data(), statements.size() * sizeof(handle<node>));
				preceding_branch->children[1] = branch_node; // point to the next branch
				break;
			}
			else {
				NOT_IMPLEMENTED();
			}
		}

		return entry;
	}

	auto parser::parse_if_statement() -> utility::result<handle<node>> {
		// expect 'IF ( condition ) { statements }'
		EXPECT_CURRENT_TOKEN(token_type::IF);
		EXPECT_NEXT_TOKEN(token_type::LEFT_PARENTHESIS);

		// parse the condition
		m_tokens.next(); // prime the expression token
		TRY(const handle<node> condition, parse_expression());

		EXPECT_CURRENT_TOKEN(token_type::RIGHT_PARENTHESIS);

		// parse inner statements 
		m_tokens.next(); // prime the left brace
		TRY(const std::vector<handle<node>> statements, parse_statement_block());

		const handle<node> branch_node = create_node<utility::empty_property>(node_type::CONDITIONAL_BRANCH, statements.size() + 2, nullptr);

		branch_node->children[0] = condition; // condition
		branch_node->children[1] = nullptr;   // false condition target

		// copy all child statements
		std::memcpy(branch_node->children.get_data() + 2, statements.data(), statements.size() * sizeof(handle<node>));

		return branch_node;
	}

	auto parser::parse_identifier_statement() -> utility::result<handle<node>> {
		// parse a statement which starts with an identifier
		// expect '(NAMESPACES) IDENTIFIER (ARRAY_ACCESS)'

		// parse a namespace, if there is one
		TRY(const std::vector<utility::string_table_key> namespaces, parse_namespaces());

		if (peek_is_function_call()) {
			// return early and don't allow assignments to function results (yet)
			return parse_function_call(namespaces);
		}

		// parse a variable value
		TRY(const handle<node> variable, parse_variable_access());

		handle<node> array_access = nullptr;

		// parse an array access
		if(m_tokens.get_current_token() == token_type::LEFT_BRACKET) {
			TRY(array_access, parse_array_access());
		}
		
		// check if we're assigning anything
		// TODO: replace by a store op
		if (m_tokens.get_current_token() == token_type::EQUALS_SIGN) {
			TRY(const handle<node> assignment_node, parse_assignment());

			if(array_access) {
				// assignment to an array access
				array_access->children[0] = variable;
				assignment_node->children[0] = array_access;
			}
			else {
				assignment_node->children[0] = variable;
			}

			return assignment_node;
		}

		if(array_access) {
			// plain array access
			array_access->children[0] = variable;
			return array_access;
		}

		return variable;
	}

	auto parser::parse_negative_expression() -> utility::result<handle<node>> {
		EXPECT_CURRENT_TOKEN(token_type::MINUS_SIGN);
		const handle<token_location> location = m_tokens.get_current_token_location();
		m_tokens.next(); // prime the first expression token

		TRY(const handle<node> expression_node, parse_expression());
		const handle<node> negation_node = create_node<ast_literal>(node_type::NUMERICAL_LITERAL, 0, location);

		ast_literal& literal = negation_node->get<ast_literal>();
		literal.value_key = m_context.syntax.strings.insert("-1");
		literal.type = { data_type::I32, 0 };

		// negate the expression
		return create_binary_expression(node_type::OPERATOR_MULTIPLY, negation_node, expression_node);
	}

	auto parser::parse_function_call(const std::vector<utility::string_table_key>& namespaces) -> utility::result<handle<node>> {
		// expect 'IDENTIFIER ( PARAMETER, ... , PARAMETER )'
		EXPECT_CURRENT_TOKEN(token_type::IDENTIFIER);

		const handle<token_location> call_location = m_tokens.get_current_token_location();
		const utility::string_table_key identifier_key = m_tokens.get_current().symbol_key;
		std::vector<handle<node>> parameters;

		EXPECT_NEXT_TOKEN(token_type::LEFT_PARENTHESIS);
		m_tokens.next();

		// parse call parameters
		if(m_tokens.get_current_token() != token_type::RIGHT_PARENTHESIS) {
			while(true) {
				TRY(handle<node> expression_node, parse_expression());
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
		const handle<node> call_node = create_node<ast_function_call>(
			node_type::FUNCTION_CALL, parameters.size(), call_location
		);

		// copy over function parameters
		std::memcpy(call_node->children.get_data(), parameters.data(), parameters.size() * sizeof(handle<node>));

		// initialize the callee
		ast_function_call& function = call_node->get<ast_function_call>();
		function.signature.identifier_key = identifier_key;
		function.namespaces = namespaces;

		m_tokens.next();
		return call_node;
	}

	auto parser::parse_variable_declaration() -> utility::result<handle<node>> {
		// expect 'TYPE IDENTIFIER'
		const handle<token_location> location = m_tokens.get_current_token_location();
		handle<node> assigned_value = nullptr;

		// parse the variable type
		TRY(const data_type type, parse_type());

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
		handle<node> variable_node;

		if(assigned_value) {
			variable_node = create_node<ast_variable>(node_type::VARIABLE_DECLARATION, 1, location);
			variable_node->children[0] = assigned_value;
		}
		else {
			variable_node = create_node<ast_variable>(node_type::VARIABLE_DECLARATION, 0, location);
		}

		// initialize the variable
		auto& variable = variable_node->get<ast_variable>();
		variable.identifier_key = identifier_key;
		variable.type = type;

		return variable_node;
	}

	auto parser::parse_variable_access() -> utility::result<handle<node>> {
		// expect 'IDENTIFIER'
		EXPECT_CURRENT_TOKEN(token_type::IDENTIFIER);
		const handle<token_location> location = m_tokens.get_current_token_location();

		// create the access node
		const handle<node> variable_node = create_node<ast_variable>(node_type::VARIABLE_ACCESS, 0, location);

		// initialize the variable
		auto& variable = variable_node->get<ast_variable>();
		variable.identifier_key = m_tokens.get_current().symbol_key;

		m_tokens.next();
		return variable_node;
	}

	auto parser::parse_array_access() -> utility::result<handle<node>> {
		// parses an array index access
		// expect '[index expression]'
		// ie. [12, 3, index]
		const handle<token_location> location = m_tokens.get_current_token_location();
		std::vector<handle<node>> index_expressions;

		while(true) {
			m_tokens.next(); // prime the first expression token

			TRY(const handle<node> index, parse_expression());
			index_expressions.push_back(index);

			if(m_tokens.get_current_token() == token_type::RIGHT_BRACKET) {
				break;
			}

			EXPECT_CURRENT_TOKEN(token_type::COMMA);
		}

		const handle<node> array_access = create_node<ast_array_access>(
			node_type::ARRAY_ACCESS, index_expressions.size() + 1, location
		);

		std::memcpy(array_access->children.get_data() + 1, index_expressions.data(), index_expressions.size() * sizeof(handle<node>));

		m_tokens.next(); // prime the next token
		return array_access;
	}

	auto parser::parse_sizeof() -> utility::result<handle<node>> {
		// expect 'SIZEOF ( type )'
		EXPECT_CURRENT_TOKEN(token_type::SIZEOF);
		const handle<token_location> location = m_tokens.get_current_token_location();

		EXPECT_NEXT_TOKEN(token_type::LEFT_PARENTHESIS);

		// parse the type
		m_tokens.next(); // prime the type token
		TRY(const data_type type, parse_type());

		EXPECT_CURRENT_TOKEN(token_type::RIGHT_PARENTHESIS);

		// create the sizeof node
		const handle<node> sizeof_node = create_node<ast_sizeof>(node_type::SIZEOF, 0, location);
		sizeof_node->get<ast_sizeof>().type = type;

		m_tokens.next();
		return sizeof_node;
	}

	auto parser::parse_assignment() -> utility::result<handle<node>> {
		// expect '= expression'
		EXPECT_CURRENT_TOKEN(token_type::EQUALS_SIGN);

		const handle<token_location> location = m_tokens.get_current_token_location();
		m_tokens.next(); 

		// create the assignment node
		const handle<node> assignment_node = create_node<utility::empty_property>(
			node_type::VARIABLE_ASSIGNMENT, 2, location
		);

		// handle the assigned expression
		TRY(assignment_node->children[1], parse_expression()); 

		return assignment_node;
	}

	auto parser::parse_expression() -> utility::result<handle<node>> {
		return parse_logical_conjunction();
	}

	auto parser::parse_logical_conjunction() -> utility::result<handle<node>> {
		TRY(const handle<node> left_node, parse_logical_disjunction());
		return left_node;
	}

	auto parser::parse_logical_disjunction() -> utility::result<handle<node>> {
		TRY(const handle<node> left_node, parse_comparison());
		return left_node;
	}

	auto parser::parse_comparison() -> utility::result<handle<node>> {
		TRY(const handle<node> left_node, parse_term());
		return left_node;
	}

	auto parser::parse_term() -> utility::result<handle<node>> {
		TRY(handle<node> left_node, parse_factor());
		token operation = m_tokens.get_current_token();

		while (
			operation == token_type::PLUS_SIGN ||
			operation == token_type::MINUS_SIGN
		) {
			m_tokens.next(); // prime the term

			TRY(const handle<node> right_node, parse_factor());
			node_type operator_type = node_type::UNKNOWN;

			switch (operation) {
				case token_type::PLUS_SIGN:  operator_type = node_type::OPERATOR_ADD; break;
				case token_type::MINUS_SIGN: operator_type = node_type::OPERATOR_SUBTRACT; break;
				default: PANIC("unhandled term case for token '{}'", operation.to_string());
			}

			left_node = create_binary_expression(operator_type, left_node, right_node);
			operation = m_tokens.get_current_token();
		}

		return left_node;
	}

	auto parser::parse_factor() -> utility::result<handle<node>> {
		TRY(handle<node> left_node, parse_primary());
		token operation = m_tokens.get_current_token();

		while (
			operation == token_type::ASTERISK ||
			operation == token_type::SLASH ||
			operation == token_type::MODULO
		) {
			m_tokens.next(); // prime the primary

			TRY(const handle<node> right_node, parse_primary());
			node_type operator_type = node_type::UNKNOWN;

			switch (operation) {
				case token_type::ASTERISK: operator_type = node_type::OPERATOR_MULTIPLY; break;
				case token_type::SLASH:    operator_type = node_type::OPERATOR_DIVIDE; break;
				case token_type::MODULO:   operator_type = node_type::OPERATOR_MODULO; break;
				default: PANIC("unhandled factor case for token '{}'", operation.to_string());
			}

			left_node = create_binary_expression(operator_type, left_node, right_node);
			operation = m_tokens.get_current_token();
		}

		return left_node;
	}

	auto parser::parse_primary() -> utility::result<handle<node>> {
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
			case token_type::SIZEOF:             return parse_sizeof();
			case token_type::STRING_LITERAL:     return parse_string_literal();
			case token_type::CHARACTER_LITERAL:  return parse_character_literal();
			case token_type::MINUS_SIGN:         return parse_negative_expression();
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

	auto parser::parse_explicit_cast() -> utility::result<handle<node>> {
		// expect 'CAST < TYPE >'
		EXPECT_CURRENT_TOKEN(token_type::CAST);
		const handle<token_location> location = m_tokens.get_current_token_location();

		EXPECT_NEXT_TOKEN(token_type::LESS_THAN);

		// parse the target type
		m_tokens.next(); // prime the type token
		TRY(const data_type target_type, parse_type());

		EXPECT_CURRENT_TOKEN(token_type::GREATER_THAN);
		EXPECT_NEXT_TOKEN(token_type::LEFT_PARENTHESIS);

		// parse the expression we want to cast
		m_tokens.next(); // prime the statement
		TRY(const handle<node> value_to_cast, parse_expression());

		EXPECT_CURRENT_TOKEN(token_type::RIGHT_PARENTHESIS);
		m_tokens.next(); // prime the next token (probably a SEMICOLON)

		// create the cast node
		const handle<node> cast_node = create_node<ast_cast>(node_type::CAST, 1, location);
		cast_node->children[0] = value_to_cast;

		ast_cast& cast = cast_node->get<ast_cast>();
		cast.target_type = target_type;

		return cast_node;
	}

	auto parser::parse_type() -> utility::result<data_type> {
		// expect '(TYPE | IDENTIFIER)* ... *'
		const token type_token = m_tokens.get_current_token();
		u8 pointer_level = 0;

		if(!type_token.is_type()) {
			return error::emit(
				error::code::UNEXPECTED_TOKEN,
				m_tokens.get_current_token_location(),
				type_token.to_string()
			);
		}

		// parse pointers
		while(m_tokens.peek_next_token() == token_type::ASTERISK) {
			m_tokens.next();
			pointer_level++;
		}

		m_tokens.next();
		return data_type{ type_token, pointer_level };
	}

	auto parser::parse_numerical_literal() -> utility::result<handle<node>> {
		// expect 'NUMERICAL_LITERAL'
		const token literal_token = m_tokens.get_current_token();
		const handle<token_location> location = m_tokens.get_current_token_location();

		if(!literal_token.is_numerical_literal()) {
			return error::emit(
				error::code::UNEXPECTED_NON_NUMERICAL,
				m_tokens.get_current_token_location(),
				literal_token.to_string()
			);
		}

		data_type::data_type_base base = data_type::UNKNOWN;

		// these types won't be used most of the time, but its nice to hav a place we can
		// fall back to
		switch (literal_token) {
			case token_type::UNSIGNED_LITERAL:    base = data_type::U32; break;
			//case lex::token_type::F32_LITERAL:         base = data_type::F32; break;
			//case lex::token_type::F64_LITERAL:         base = data_type::F64; break;
			case token_type::SIGNED_LITERAL:
			case token_type::HEXADECIMAL_LITERAL:
			case token_type::BINARY_LITERAL:      base = data_type::I32; break;
			default: PANIC("unhandled literal to data type conversion '{}'", literal_token.to_string());
		}

		// create the literal node
		const handle<node> literal_node = create_node<ast_literal>(node_type::NUMERICAL_LITERAL, 0, location);

		// initialize the literal
		auto& literal = literal_node->get<ast_literal>();
		literal.value_key = m_tokens.get_current().symbol_key;
		literal.type = { base, 0 };

		m_tokens.next();
		return literal_node;
	}

	auto parser::parse_character_literal() -> utility::result<handle<node>> {
		// expect 'CHARACTER_LITERAL'
		EXPECT_CURRENT_TOKEN(token_type::CHARACTER_LITERAL);
		const handle<token_location> location = m_tokens.get_current_token_location();

		// create the string node
		const handle<node> char_node = create_node<ast_literal>(node_type::CHARACTER_LITERAL, 0, location);

		// initialize the literal
		auto& literal = char_node->get<ast_literal>();
		literal.value_key = m_tokens.get_current().symbol_key;
		literal.type = { data_type::CHAR, 0 }; // char

		m_tokens.next();
		return char_node;
	}

	auto parser::parse_string_literal() -> utility::result<handle<node>> {
		// expect 'STRING_LITERAL'
		EXPECT_CURRENT_TOKEN(token_type::STRING_LITERAL);
		const handle<token_location> location = m_tokens.get_current_token_location();

		// create the string node
		const handle<node> string_node = create_node<ast_literal>(node_type::STRING_LITERAL, 0, location);

		// initialize the literal
		auto& literal = string_node->get<ast_literal>();
		literal.value_key = m_tokens.get_current().symbol_key;
		literal.type = { data_type::CHAR, 1 }; // char*

		m_tokens.next();
		return string_node;
	}

	auto parser::parse_bool_literal() -> utility::result<handle<node>> {
		// expect 'BOOL_LITERAL_TRUE | BOOL_LITERAL_FALSE'
		const handle<token_location> location = m_tokens.get_current_token_location();

		ASSERT(
			m_tokens.get_current_token() == token_type::BOOL_LITERAL_FALSE ||
			m_tokens.get_current_token() == token_type::BOOL_LITERAL_TRUE, 
			"invalid token - expected an a boolean literal"
		);

		// create the bool node
		const handle<node> bool_node = create_node<ast_bool_literal>(node_type::BOOL_LITERAL, 0, location);

		// initialize the literal
		bool_node->get<ast_bool_literal>().value = m_tokens.get_current_token() == token_type::BOOL_LITERAL_TRUE;

		m_tokens.next();
		return bool_node;
	}

	auto parser::parse_null_literal() -> utility::result<handle<node>> {
		// expect 'NULL_LITERAL'
		const handle<token_location> location = m_tokens.get_current_token_location();
		EXPECT_CURRENT_TOKEN(token_type::NULL_LITERAL);

		const handle<node> null_node = create_node<utility::empty_property>(node_type::NULL_LITERAL, 0, location);

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

	auto parser::parse_namespaces() -> utility::result<std::vector<utility::string_table_key>> {
		// parses a set of contiguous namespace directives
		// expect 'NAMESPACE :: ... NAMESPACE ::'

		std::vector<utility::string_table_key> namespaces;

		while (peek_is_namespace_access()) {
			// expect 'IDENTIFIER ::'
			EXPECT_CURRENT_TOKEN(token_type::IDENTIFIER);
			namespaces.push_back(m_tokens.get_current().symbol_key);

			// sanity checks 
			EXPECT_NEXT_TOKEN(token_type::COLON);
			EXPECT_NEXT_TOKEN(token_type::COLON);

			m_tokens.next(); // prime the next token
		}

		return std::move(namespaces);
	}

	auto parser::peek_is_function_definition() -> bool {
		// peek 'TYPE IDENTIFIER ('
		// starting token is presumably the beginning token of a type
		if (!is_current_token_type()) {
			return false;
		}

		if (m_tokens.peek_token() != token_type::IDENTIFIER) {
			m_tokens.synchronize();
			return false;
		}

		const bool res = m_tokens.peek_token() == token_type::LEFT_PARENTHESIS;
		m_tokens.synchronize();
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

	auto parser::peek_is_variable_declaration() -> bool {
		// peek 'TYPE * ... * IDENTIFIER'
		if (!is_current_token_type()) {
			return false;
		}

		// consume pointers
		while(m_tokens.peek_token() == token_type::ASTERISK) {}

		const bool res = m_tokens.get_current_peek_token() == token_type::IDENTIFIER;
		m_tokens.synchronize();
		return res;
	}

	auto parser::peek_is_namespace_access() -> bool {
		// peek 'IDENTIFIER::'
		if(m_tokens.get_current_token() != token_type::IDENTIFIER) {
			return false;
		}

		if (m_tokens.peek_token() != token_type::COLON) {
			m_tokens.synchronize();
			return false;
		}

		const bool res = m_tokens.peek_token() == token_type::COLON;
		m_tokens.synchronize();
		return res;
	}

	auto parser::create_binary_expression(node_type type, handle<node> left, handle<node> right) const -> handle<node> {
		return m_context.syntax.ast.create_binary_expression(type, left, right);
	}

	auto parser::parse_identifier_expression() -> utility::result<handle<node>> {
		// parse an expression which starts with an identifier
		// expect '(NAMESPACES) IDENTIFIER (ARRAY_ACCESS)'

		// parse a namespace, if there is one
		TRY(const std::vector<utility::string_table_key> namespaces, parse_namespaces());

		if (peek_is_function_call()) {
			// return early and don't allow assignments to function results (yet)
			return parse_function_call(namespaces);
		}

		const handle<token_location> location = m_tokens.get_current_token_location();
		const handle<node> load_node = create_node<ast_load>(node_type::LOAD, 1, location);

		// parse a variable value
		TRY(const handle<node> variable, parse_variable_access());

		// parse an array access
		if(m_tokens.get_current_token() == token_type::LEFT_BRACKET) {
			TRY(const handle<node> array_access, parse_array_access());

			array_access->children[0] = variable;
			load_node->children[0] = array_access;
		}
		else {
			load_node->children[0] = variable;
		}

		return load_node;
	}
} // namespace sigma
