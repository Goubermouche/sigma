#include "parser.h"
#include "compiler/compiler/compilation_context.h"

#define EXPECT_CURRENT(__token)                                             \
do {                                                                        \
	if (m_tokens.get_current_token() != (__token)) {                          \
		return utility::error::create(                                          \
			utility::error::code::UNEXPECTED_TOKEN_WITH_EXPECTED,                 \
			token(__token).to_string(), m_tokens.get_current_token().to_string()  \
		);                                                                      \
	}                                                                         \
} while(false)

#define EXPECT_NEXT(__token) \
m_tokens.next();             \
EXPECT_CURRENT((__token))

namespace sigma {
	parser::parser(frontend_context& context) : m_context(context), m_tokens(context.tokens) {}

	auto parser::parse(frontend_context& context) -> utility::result<void> {
		return parser(context).parse();
	}

	auto parser::parse() -> utility::result<void> {
		// TODO: handle peeks looking past EOF
		// TODO: manage local context (ie. function body, loop body, if body etc), probably use a stack

		while (m_tokens.get_current_token() != token_type::END_OF_FILE) {
			handle<node> node;
			if (peek_is_function_definition()) {
				TRY(node, parse_function_declaration());
			}
			else {
				NOT_IMPLEMENTED();
			}

			m_context.syntax.ast.add_node(node);
			m_tokens.next();
		}

		return SUCCESS;
	}

	auto parser::parse_function_declaration() -> utility::result<handle<node>> {
		TRY(const data_type return_type, parse_type());
		std::vector<named_data_type> parameters;

		// parse the function identifier
		EXPECT_NEXT(token_type::IDENTIFIER);
		const utility::string_table_key identifier_key = m_tokens.get_current().symbol_key;

		m_tokens.next(); // LEFT_PARENTHESIS (guaranteed)

		// parse function parameters
		while (m_tokens.peek_next_token() != token_type::RIGHT_PARENTHESIS) {
			// prime the type token
			m_tokens.next();
			TRY(data_type parameter_type, parse_type());

			// prime the identifier
			EXPECT_NEXT(token_type::IDENTIFIER);
			const auto parameter_identifier_key = m_tokens.get_current().symbol_key;
			parameters.emplace_back(parameter_type, parameter_identifier_key);

			if (m_tokens.peek_next_token() == token_type::COMMA) {
				EXPECT_NEXT(token_type::COMMA);
			}
		}

		// parse the function body
		EXPECT_NEXT(token_type::RIGHT_PARENTHESIS);
		TRY(const std::vector<handle<node>> statements, parse_statement_block());

		const handle<node> function_node = m_context.syntax.ast.create_node<function_signature>(node_type::FUNCTION_DECLARATION, statements.size());

		auto& function = function_node->get<sigma::function_signature>();
		function.return_type = return_type;
		function.identifier_key = identifier_key;
		function.parameter_types = utility::slice<named_data_type>(m_context.syntax.ast.get_allocator(), parameters.size());

		// copy all statement pointers over to the memory arena
		std::memcpy(function_node->children.get_data(), statements.data(), statements.size() * sizeof(handle<node>));
		std::memcpy(function.parameter_types.get_data(), parameters.data(), parameters.size() * sizeof(named_data_type));

		return function_node;
	}

	auto parser::parse_statement_block() -> utility::result<std::vector<handle<node>>> {
		EXPECT_NEXT(token_type::LEFT_BRACE);
		std::vector<handle<node>> statements;

		while (m_tokens.peek_next_token() != token_type::RIGHT_BRACE) {
			m_tokens.next(); // prime the first statement keyword
			TRY(handle<node> statement, parse_statement());
			statements.push_back(statement);
		}

		EXPECT_NEXT(token_type::RIGHT_BRACE);
		return statements;
	}

	auto parser::parse_statement() -> utility::result<handle<node>> {
		handle<node> result;

		if (peek_is_variable_declaration()) {
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
					return utility::error::create(utility::error::code::UNEXPECTED_TOKEN, m_tokens.get_current_token().to_string());
				}
			}
		}

		// expect a trailing semicolon
		EXPECT_NEXT(token_type::SEMICOLON);
		return result;
	}

	auto parser::parse_return_statement() -> utility::result<handle<node>> {
		// first token is the RET keyword
		// allow return statements without any expressions
		if (m_tokens.peek_next_token() == token_type::SEMICOLON) {
			NOT_IMPLEMENTED();
			return nullptr;
		}

		m_tokens.next(); // prime the first expression token

		const handle<node> return_node = m_context.syntax.ast.create_node<return_statement>(node_type::RETURN, 1);
		TRY(return_node->children[0], parse_expression());
		return return_node;
	}

	auto parser::parse_if_else_statement_block() -> utility::result<handle<node>> {
		// the first token is an IF keyword
		TRY(const handle<node> entry, parse_if_statement());
		handle<node> branch_before = entry;

		// parse succeeding if else statements
		while (m_tokens.peek_next_token() == token_type::ELSE) {
			m_tokens.next();

			if (m_tokens.peek_next_token() == token_type::IF) {
				// if else
				EXPECT_NEXT(token_type::IF);
				TRY(const handle<node> branch, parse_if_statement());

				branch_before->children[1] = branch; // point to the next branch
				branch_before = branch;
			}
			else if (m_tokens.peek_next_token() == token_type::LEFT_BRACE) {
				// else
				TRY(const std::vector<handle<node>> statements, parse_statement_block());

				const handle<node> branch_node = m_context.syntax.ast.create_node<utility::empty_property>(node_type::BRANCH, statements.size());
				std::memcpy(branch_node->children.get_data(), statements.data(), statements.size() * sizeof(handle<node>));
				branch_before->children[1] = branch_node; // point to the next branch
				break;
			}
		}

		return entry;
	}

	auto parser::parse_if_statement() -> utility::result<handle<node>> {
		// parses a statement in the following format:
		// IF ( condition ) { statements }

		EXPECT_NEXT(token_type::LEFT_PARENTHESIS);
		m_tokens.next(); // prime the expression token

		TRY(const handle<node> condition, parse_expression());

		EXPECT_NEXT(token_type::RIGHT_PARENTHESIS);
		TRY(const std::vector<handle<node>> statements, parse_statement_block());

		const handle<node> branch_node = m_context.syntax.ast.create_node<utility::empty_property>(node_type::CONDITIONAL_BRANCH, statements.size() + 2);

		branch_node->children[0] = condition; // condition
		branch_node->children[1] = nullptr;   // false condition target

		std::memcpy(branch_node->children.get_data() + 2, statements.data(), statements.size() * sizeof(handle<node>));

		return branch_node;
	}

	auto parser::parse_identifier_statement() -> utility::result<handle<node>> {
		// parse a statement which starts with an identifier

		if (peek_is_function_call()) {
			return parse_function_call();
		}
		else {
			TRY(const handle<node> variable, parse_variable_access());

			// check if we're assigning anything
			if (m_tokens.peek_next_token() == token_type::EQUALS_SIGN) {
				EXPECT_NEXT(token_type::EQUALS_SIGN);
				TRY(const handle<node> assignment, parse_assignment());

				assignment->children[0] = variable;
				return assignment;
			}
			else {
				return variable;
			}
		}
	}

	auto parser::parse_negative_expression() -> utility::result<handle<node>> {
		EXPECT_CURRENT(token_type::MINUS_SIGN);
		m_tokens.next(); // prime the first expression token

		TRY(const handle<node> expression_node, parse_expression());
		const handle<node> negation_node = m_context.syntax.ast.create_node<literal>(node_type::NUMERICAL_LITERAL, 0);

		auto& literal = negation_node->get<sigma::literal>();
		literal.value_key = m_context.syntax.string_table.insert("-1");
		literal.type = { data_type::I32, 0 };

		// negate the expression
		return m_context.syntax.ast.create_binary_expression(node_type::OPERATOR_MULTIPLY, negation_node, expression_node);
	}

	auto parser::parse_function_call() -> utility::result<handle<node>> {
		ASSERT(m_tokens.get_current_token() == token_type::IDENTIFIER, "invalid token - expected a 'MINUS_SIGN'");

		const utility::string_table_key identifier_key = m_tokens.get_current().symbol_key;
		std::vector<handle<node>> parameters;

		EXPECT_NEXT(token_type::LEFT_PARENTHESIS); 

		// parse call parameters
		while (m_tokens.peek_next_token() != token_type::RIGHT_PARENTHESIS) {
			m_tokens.next(); // prime the expression token
			TRY(handle<node> expression, parse_expression());
			parameters.push_back(expression);

			if (m_tokens.peek_next_token() == token_type::COMMA) {
				EXPECT_NEXT(token_type::COMMA);
			}
		}

		EXPECT_NEXT(token_type::RIGHT_PARENTHESIS);

		// create the call node
		const handle<node> call_node = m_context.syntax.ast.create_node<function_signature>(node_type::FUNCTION_CALL, parameters.size());
		std::memcpy(call_node->children.get_data(), parameters.data(), parameters.size() * sizeof(handle<node>));
		call_node->get<function_signature>().identifier_key = identifier_key;

		return call_node;
	}

	auto parser::parse_variable_declaration() -> utility::result<handle<node>> {
		// first token is the type
		TRY(const data_type type, parse_type());
		handle<node> assigned_value = nullptr;

		EXPECT_NEXT(token_type::IDENTIFIER);
		const auto identifier_key = m_tokens.get_current().symbol_key;

		if (m_tokens.peek_next_token() == token_type::EQUALS_SIGN) {
			EXPECT_NEXT(token_type::EQUALS_SIGN);
			m_tokens.next(); // prime the first expression token

			TRY(assigned_value, parse_expression());
		}

		// create the variable node
		const handle<node> variable_node = m_context.syntax.ast.create_node<variable>(node_type::VARIABLE_DECLARATION, assigned_value ? 1 : 0);
		auto& variable = variable_node->get<sigma::variable>();
		variable.type = type;
		variable.identifier_key = identifier_key;

		if (assigned_value) {
			variable_node->children[0] = assigned_value;
		}

		return variable_node;
	}

	auto parser::parse_variable_access() -> utility::result<handle<node>> {
		EXPECT_CURRENT(token_type::IDENTIFIER);

		// create the access node
		const handle<node> access_node = m_context.syntax.ast.create_node<variable>(node_type::VARIABLE_ACCESS, 0);
		access_node->get<variable>().identifier_key = m_tokens.get_current().symbol_key;

		return access_node;
	}

	auto parser::parse_assignment() -> utility::result<handle<node>> {
		EXPECT_CURRENT(token_type::EQUALS_SIGN);

		// handle the assigned expression
		m_tokens.next(); // prime the expression token
		TRY(const handle<node> expression, parse_expression());

		// create the assignment node
		const handle<node> assignment_node = m_context.syntax.ast.create_node<utility::empty_property>(node_type::VARIABLE_ASSIGNMENT, 2);
		assignment_node->children[1] = expression;

		return assignment_node;
	}

	auto parser::parse_expression() -> utility::result<handle<node>> {
		return parse_logical_conjunction();
	}

	auto parser::parse_logical_conjunction() -> utility::result<handle<node>> {
		TRY(const handle<node> left, parse_logical_disjunction());
		return left;
	}

	auto parser::parse_logical_disjunction() -> utility::result<handle<node>> {
		TRY(const handle<node> left, parse_comparison());
		return left;
	}

	auto parser::parse_comparison() -> utility::result<handle<node>> {
		TRY(const handle<node> left, parse_term());
		return left;
	}

	auto parser::parse_term() -> utility::result<handle<node>> {
		TRY(handle<node> left, parse_factor());
		token operation = m_tokens.peek_next_token();

		while (
			operation == token_type::PLUS_SIGN ||
			operation == token_type::MINUS_SIGN
		) {
			m_tokens.next(); // consume the operator
			m_tokens.next(); // prime the term

			TRY(const handle<node> right, parse_factor());
			node_type operator_type = node_type::UNKNOWN;

			switch (operation) {
				case token_type::PLUS_SIGN:  operator_type = node_type::OPERATOR_ADD; break;
				case token_type::MINUS_SIGN: operator_type = node_type::OPERATOR_SUBTRACT; break;
				default: PANIC("unhandled term case for token '{}'", operation.to_string());
			}

			left = m_context.syntax.ast.create_binary_expression(operator_type, left, right);
			operation = m_tokens.peek_next_token();
		}

		return left;
	}

	auto parser::parse_factor() -> utility::result<handle<node>> {
		TRY(handle<node> left, parse_primary());
		token operation = m_tokens.peek_next_token();

		while (
			operation == token_type::ASTERISK ||
			operation == token_type::SLASH ||
			operation == token_type::MODULO
		) {
			m_tokens.next(); // consume the operator
			m_tokens.next(); // prime the primary

			TRY(const handle<node> right, parse_primary());
			node_type operator_type = node_type::UNKNOWN;

			switch (operation) {
				case token_type::ASTERISK: operator_type = node_type::OPERATOR_MULTIPLY; break;
				case token_type::SLASH:    operator_type = node_type::OPERATOR_DIVIDE; break;
				case token_type::MODULO:   operator_type = node_type::OPERATOR_MODULO; break;
				default: PANIC("unhandled factor case for token '{}'", operation.to_string());
			}

			left = m_context.syntax.ast.create_binary_expression(operator_type, left, right);
			operation = m_tokens.peek_next_token();
		}

		return left;
	}

	auto parser::parse_primary() -> utility::result<handle<node>> {
		if (m_tokens.get_current_token().is_numerical_literal()) {
			return parse_numerical_literal();
		}

		switch (m_tokens.get_current_token()) {
			case token_type::IDENTIFIER:         return parse_identifier_statement();
			case token_type::STRING_LITERAL:     return parse_string_literal();
			case token_type::MINUS_SIGN:         return parse_negative_expression();
			case token_type::BOOL_LITERAL_TRUE:
			case token_type::BOOL_LITERAL_FALSE: return parse_bool_literal();
			default: utility::error::create(utility::error::code::UNEXPECTED_TOKEN, m_tokens.get_current_token().to_string());
		}

		return nullptr;
	}

	auto parser::parse_type() const -> utility::result<data_type> {
		const token type_token = m_tokens.get_current_token();

		if(!type_token.is_type()) {
			return utility::error::create(utility::error::code::INVALID_TYPE_TOKEN, type_token.to_string());
		}

		return data_type{ type_token, 0 };
	}

	auto parser::parse_numerical_literal() -> utility::result<handle<node>> {
		const token literal_token = m_tokens.get_current_token();

		if(!literal_token.is_numerical_literal()) {
			return utility::error::create(utility::error::code::UNEXPECTED_NON_NUMERICAL, literal_token.to_string());
		}

		// get a symbol key to the literal value
		data_type::data_type_base base = data_type::UNKNOWN;

		// these types won't be used most of the time, but its nice to hav a place we can
		// fall back to
		switch (literal_token) {
			case token_type::SIGNED_LITERAL:      base = data_type::I32; break;
			//case lex::token_type::UNSIGNED_LITERAL:    base = data_type::U32; break;
			//case lex::token_type::F32_LITERAL:         base = data_type::F32; break;
			//case lex::token_type::F64_LITERAL:         base = data_type::F64; break;
			case token_type::HEXADECIMAL_LITERAL: base = data_type::I32; break;
			case token_type::BINARY_LITERAL:      base = data_type::I32; break;
			default: PANIC("unhandled literal to data type conversion '{}'", literal_token.to_string());
		}

		// create the literal node
		const handle<node> literal_node = m_context.syntax.ast.create_node<literal>(node_type::NUMERICAL_LITERAL, 0);
		auto& literal = literal_node->get<sigma::literal>();
		literal.value_key = m_tokens.get_current().symbol_key;
		literal.type = { base, 0 };

		return literal_node;
	}

	auto parser::parse_string_literal() -> utility::result<handle<node>> {
		EXPECT_CURRENT(token_type::STRING_LITERAL);

		// create the string node
		const handle<node> string_node = m_context.syntax.ast.create_node<literal>(node_type::STRING_LITERAL, 0);
		auto& literal = string_node->get<sigma::literal>();
		literal.value_key = m_tokens.get_current().symbol_key;
		literal.type = { data_type::CHAR, 1 }; // char*

		return string_node;
	}

	auto parser::parse_bool_literal() -> utility::result<handle<node>> {
		ASSERT(
			m_tokens.get_current_token() == token_type::BOOL_LITERAL_FALSE || m_tokens.get_current_token() == token_type::BOOL_LITERAL_TRUE, 
			"invalid token - expected an a boolean literal"
		);

		// create the bool node
		const handle<node> bool_node = m_context.syntax.ast.create_node<bool_literal>(node_type::BOOL_LITERAL, 0);
		bool_node->get<bool_literal>().value = m_tokens.get_current_token() == token_type::BOOL_LITERAL_TRUE;

		return bool_node;
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

	auto parser::peek_is_function_definition() -> bool {
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

	auto parser::peek_is_function_call() const -> bool {
		if (m_tokens.get_current_token() != token_type::IDENTIFIER) {
			return false;
		}

		// TODO: this will have to handle generics when time comes
		return m_tokens.peek_next_token() == token_type::LEFT_PARENTHESIS;
	}

	auto parser::peek_is_variable_declaration() const -> bool {
		if (!is_current_token_type()) {
			return false;
		}

		return m_tokens.peek_next_token() == token_type::IDENTIFIER;
	}
} // namespace sigma
