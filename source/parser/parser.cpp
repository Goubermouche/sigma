#include "parser.h"
#include <compiler/compiler/compilation_context.h>

namespace sigma {
	parser::parser(compilation_context& context)
		: m_context(context), m_tokens(context.tokens) {}

	auto parser::parse(compilation_context& context) -> abstract_syntax_tree {
		return parser(context).parse();
	}

	auto parser::parse() -> abstract_syntax_tree {
		// TODO: handle peeks looking past EOF
		// TODO: manage local context (ie. function body, loop body, if body etc), probably use
		//       a stack

		while (m_tokens.get_current_token() != token_type::END_OF_FILE) {
			if (peek_is_function_definition()) {
				m_ast.add_node(parse_function_declaration());
			}
			else {
				NOT_IMPLEMENTED();
			}

			m_tokens.next();
		}

		return std::move(m_ast);
	}

	auto parser::parse_function_declaration() -> handle<node> {
		const data_type return_type = parse_type();
		std::vector<named_data_type> parameters;

		// IDENTIFIER (guaranteed)
		m_tokens.next();
		const auto identifier_key = m_tokens.get_current().symbol_key;

		// LEFT_PARENTHESIS (guaranteed)
		m_tokens.next();

		// parse function parameters
		while (m_tokens.peek_next_token() != token_type::RIGHT_PARENTHESIS) {
			// prime the type token
			m_tokens.next();
			data_type parameter_type = parse_type();

			// prime the identifier
			m_tokens.expect_next(token_type::IDENTIFIER);
			const auto parameter_identifier_key = m_tokens.get_current().symbol_key;

			parameters.emplace_back(parameter_type, parameter_identifier_key);

			if (m_tokens.peek_next_token() == token_type::COMMA) {
				// COMMA (guaranteed)
				m_tokens.next();
			}
		}

		m_tokens.expect_next(token_type::RIGHT_PARENTHESIS);

		// parse the function body
		const std::vector<handle<node>> statements = parse_statement_block();

		handle<node> function_node = m_ast.create_node<function>(
			node_type::FUNCTION, statements.size()
		);

		auto& prop = function_node->get<function>();
		prop.return_type = return_type;
		prop.identifier_key = identifier_key;
		prop.parameter_types = utility::slice<named_data_type>(m_ast.get_allocator(), parameters.size());

		// copy all statement pointers over to the memory arena
		std::memcpy(
			function_node->children.get_data(),
			statements.data(),
			statements.size() * sizeof(handle<node>)
		);

		std::memcpy(
			prop.parameter_types.get_data(),
			parameters.data(),
			parameters.size() * sizeof(named_data_type)
		);

		return function_node;
	}

	auto parser::parse_statement_block() -> std::vector<handle<node>> {
		// LEFT_BRACE
		m_tokens.expect_next(token_type::LEFT_BRACE);
		std::vector<handle<node>> statements;

		while (m_tokens.peek_next_token() != token_type::RIGHT_BRACE) {
			// prime the first statement keyword
			m_tokens.next();
			statements.push_back(parse_statement());
		}

		// RIGHT_BRACE
		m_tokens.expect_next(token_type::RIGHT_BRACE);
		return statements;
	}

	auto parser::parse_statement() -> handle<node> {
		handle<node> result;

		if (peek_is_variable_declaration()) {
			result = parse_variable_declaration();
		}
		else {
			switch (m_tokens.get_current_token()) {
				case token_type::IDENTIFIER: {
					result = parse_identifier_statement();
					break;
				}
				case token_type::IF: {
					return parse_if_else_statement_block();
				}
				case token_type::RET: {
					result = parse_return_statement();
					break;
				}

				default: NOT_IMPLEMENTED();
			}
		}

		// expect a trailing semicolon
		m_tokens.expect_next(token_type::SEMICOLON);
		return result;
	}

	auto parser::parse_return_statement() -> handle<node> {
		// first token is the RET keyword
		// allow return statements without any expressions
		if (m_tokens.peek_next_token() == token_type::SEMICOLON) {
			NOT_IMPLEMENTED();
			return nullptr;
		}

		// prime the expression start token
		m_tokens.next();

		handle<node> return_node = m_ast.create_node<return_statement>(node_type::RETURN, 1);
		return_node->children[0] = parse_expression();
		return return_node;
	}

	auto parser::parse_if_else_statement_block() -> handle<node> {
		// the first token is an IF keyword
		handle<node> entry = parse_if_statement();
		handle<node> branch_before = entry;

		// parse succeeding if else statements
		while (m_tokens.peek_next_token() == token_type::ELSE) {
			m_tokens.next();

			if (m_tokens.peek_next_token() == token_type::IF) {
				// if else
				m_tokens.next();
				const handle<node> succeeding_branch = parse_if_statement();
				branch_before->children[1] = succeeding_branch; // point to the next branch
				branch_before = succeeding_branch;
			}
			else if (m_tokens.peek_next_token() == token_type::LEFT_BRACE) {
				// else
				const std::vector<handle<node>> statements = parse_statement_block();

				const handle<node> br = m_ast.create_node<utility::empty_property>(
					node_type::BRANCH, statements.size()
				);

				std::memcpy(
					br->children.get_data(), 
					statements.data(), 
					statements.size() * sizeof(handle<node>)
				);

				branch_before->children[1] = br; // point to the next branch
				branch_before = br;
				break;
			}
		}

		return entry;
	}

	auto parser::parse_if_statement() -> handle<node> {
		// parses a statement in the following format:
		// IF ( condition ) { statements }

		m_tokens.expect_next(token_type::LEFT_PARENTHESIS);
		m_tokens.next(); // prime the expression token

		const handle<node> condition = parse_expression();

		m_tokens.expect_next(token_type::RIGHT_PARENTHESIS);
		const std::vector<handle<node>> statements = parse_statement_block();

		handle<node> cond_br = m_ast.create_node<utility::empty_property>(
			node_type::CONDITIONAL_BRANCH, statements.size() + 2
		);

		cond_br->children[0] = condition; // condition
		cond_br->children[1] = nullptr;   // false condition target

		std::memcpy(
			cond_br->children.get_data() + 2, 
			statements.data(),
			statements.size() * sizeof(handle<node>)
		);

		return cond_br;
	}

	auto parser::parse_identifier_statement() -> handle<node> {
		// parse a statement which starts with an identifier

		if (peek_is_function_call()) {
			return parse_function_call();
		}
		else {
			handle<node> variable = parse_variable_access();

			// check if we're assigning anything
			if (m_tokens.peek_next_token() == token_type::EQUALS_SIGN) {
				// consume the EQUALS_SIGN
				m_tokens.next();

				handle<node> assignment = parse_assignment();
				assignment->children[0] = variable;

				return assignment;
			}
			else {
				return variable;
			}
		}
	}

	auto parser::parse_negative_expression() -> handle<node> {
		// the current token is a MINUS_SIGN
		m_tokens.next(); // prime the first expression token

		const handle<node> expression = parse_expression();
		const handle<node> negation = m_ast.create_node<literal>(
			node_type::NUMERICAL_LITERAL, 0
		);

		auto& prop = negation->get<literal>();
		prop.value_key = m_context.symbols.insert("-1");
		prop.data_type = { data_type::I32, 0 };

		// negate the expression
		return m_ast.create_binary_expression(
			node_type::OPERATOR_MULTIPLY, negation, expression
		);
	}

	auto parser::parse_function_call() -> handle<node> {
		// expects the current token to be an IDENTIFIER (guaranteed)
		const auto identifier_key = m_tokens.get_current().symbol_key;
		std::vector<handle<node>> parameters;

		// LEFT_PARENTHESIS (guaranteed)
		m_tokens.next();

		// parse call parameters
		while (m_tokens.peek_next_token() != token_type::RIGHT_PARENTHESIS) {
			// prime the expression token
			m_tokens.next();
			parameters.push_back(parse_expression());

			if (m_tokens.peek_next_token() == token_type::COMMA) {
				// COMMA (guaranteed)
				m_tokens.next();
			}
		}

		// RIGHT_PARENTHESIS
		m_tokens.expect_next(token_type::RIGHT_PARENTHESIS);

		handle<node> call = m_ast.create_node<function_call>(
			node_type::FUNCTION_CALL, parameters.size()
		);

		call->get<function_call>().callee_identifier_key = identifier_key;

		std::memcpy(
			call->children.get_data(),
			parameters.data(),
			parameters.size() * sizeof(handle<node>)
		);

		return call;
	}

	auto parser::parse_variable_declaration() -> handle<node> {
		// first token is the type
		const data_type dt = parse_type();

		// identifier
		m_tokens.expect_next(token_type::IDENTIFIER);

		const auto identifier_key = m_tokens.get_current().symbol_key;
		handle<node> assigned_value = nullptr;

		if (m_tokens.peek_next_token() == token_type::EQUALS_SIGN) {
			// consume the EQUALS_SIGN
			m_tokens.next();
			// prime the first expression token
			m_tokens.next();

			assigned_value = parse_expression();
		}

		handle<node> declaration_node = m_ast.create_node<variable>(
			node_type::VARIABLE_DECLARATION, assigned_value ? 1 : 0
		);

		auto& prop = declaration_node->get<variable>();
		prop.data_type = dt;
		prop.identifier_key = identifier_key;

		if (assigned_value) {
			declaration_node->children[0] = assigned_value;
		}

		return declaration_node;
	}

	auto parser::parse_variable_access() -> handle<node> {
		// first token is the identifier

		handle<node> access_node = m_ast.create_node<variable_access>(
			node_type::VARIABLE_ACCESS, 0
		);

		access_node->get<variable_access>().identifier_key = m_tokens.get_current().symbol_key;
		return access_node;
	}

	auto parser::parse_assignment() -> handle<node> {
		// first token is an EQUALS_SIGN
		m_tokens.next(); // prime the expression token
		const handle<node> expression = parse_expression();

		handle<node> assignment_node = m_ast.create_node<utility::empty_property>(
			node_type::VARIABLE_ASSIGNMENT, 2
		);

		assignment_node->children[1] = expression;
		return assignment_node;
	}

	auto parser::parse_expression() -> handle<node> {
		return parse_logical_conjunction();
	}

	auto parser::parse_logical_conjunction() -> handle<node> {
		handle<node> left = parse_logical_disjunction();
		return left;
	}

	auto parser::parse_logical_disjunction() -> handle<node> {
		handle<node> left = parse_comparison();
		return left;
	}

	auto parser::parse_comparison() -> handle<node> {
		handle<node> left = parse_term();
		return left;
	}

	auto parser::parse_term() -> handle<node> {
		handle<node> left = parse_factor();
		token operation = m_tokens.peek_next_token();

		while (
			operation == token_type::PLUS_SIGN ||
			operation == token_type::MINUS_SIGN
		) {
			// consume the operator
			m_tokens.next();
			// prime the term
			m_tokens.next();

			const handle<node> right = parse_factor();
			node_type operator_type = node_type::UNKNOWN;

			switch (operation) {
				case token_type::PLUS_SIGN:  operator_type = node_type::OPERATOR_ADD; break;
				case token_type::MINUS_SIGN: operator_type = node_type::OPERATOR_SUBTRACT; break;
				default: NOT_IMPLEMENTED();
			}

			left = m_ast.create_binary_expression(operator_type, left, right);
			operation = m_tokens.peek_next_token();
		}

		return left;
	}

	auto parser::parse_factor() -> handle<node> {
		handle<node> left = parse_primary();
		token operation = m_tokens.peek_next_token();

		while (
			operation == token_type::ASTERISK ||
			operation == token_type::SLASH ||
			operation == token_type::MODULO
		) {
			// consume the operator
			m_tokens.next();
			// prime the primary
			m_tokens.next();

			const handle<node> right = parse_primary();
			node_type operator_type = node_type::UNKNOWN;

			switch (operation) {
				case token_type::ASTERISK: operator_type = node_type::OPERATOR_MULTIPLY; break;
				case token_type::SLASH:    operator_type = node_type::OPERATOR_DIVIDE; break;
				case token_type::MODULO:   operator_type = node_type::OPERATOR_MODULO; break;
				default: NOT_IMPLEMENTED();
			}

			left = m_ast.create_binary_expression(operator_type, left, right);
			operation = m_tokens.peek_next_token();
		}

		return left;
	}

	auto parser::parse_primary() -> handle<node> {
		if (m_tokens.get_current_token().is_numerical_literal()) {
			return parse_numerical_literal();
		}

		switch (m_tokens.get_current_token()) {
			case token_type::IDENTIFIER:         return parse_identifier_statement();
			case token_type::STRING_LITERAL:     return parse_string_literal();
			case token_type::MINUS_SIGN:         return parse_negative_expression();
			case token_type::BOOL_LITERAL_TRUE:
			case token_type::BOOL_LITERAL_FALSE: return parse_bool_literal();
			default: NOT_IMPLEMENTED();
		}

		return nullptr;
	}

	auto parser::parse_type() const -> data_type {
		const token type_token = m_tokens.get_current_token();
		ASSERT(type_token.is_type(), "invalid type token");
		return data_type{ type_token, 0 };
	}

	auto parser::parse_numerical_literal() -> handle<node> {
		ASSERT(m_tokens.get_current_token().is_numerical_literal(), "invalid token type");

		// get a symbol key to the literal value
		data_type::base base = data_type::UNKNOWN;

		// these types won't be used most of the time, but its nice to hav a place we can
		// fall back to
		switch (m_tokens.get_current_token()) {
			case token_type::SIGNED_LITERAL:      base = data_type::I32; break;
			//case lex::token_type::UNSIGNED_LITERAL:    base = data_type::U32; break;
			//case lex::token_type::F32_LITERAL:         base = data_type::F32; break;
			//case lex::token_type::F64_LITERAL:         base = data_type::F64; break;
			case token_type::HEXADECIMAL_LITERAL: base = data_type::I32; break;
			case token_type::BINARY_LITERAL:      base = data_type::I32; break;
			default: NOT_IMPLEMENTED();
		}

		const handle<node> literal_node = m_ast.create_node<literal>(
			node_type::NUMERICAL_LITERAL, 0
		);

		auto& prop = literal_node->get<literal>();
		prop.value_key = m_tokens.get_current().symbol_key;
		prop.data_type = { base, 0 };

		return literal_node;
	}

	auto parser::parse_string_literal() -> handle<node> {
		// the first token is a STRING_LITERAL
		const handle<node> string = m_ast.create_node<literal>(node_type::STRING_LITERAL, 0);
		auto& prop = string->get<literal>();
		prop.value_key = m_tokens.get_current().symbol_key;
		prop.data_type = { data_type::CHAR, 1 }; // char*
		return string;
	}

	auto parser::parse_bool_literal() -> handle<node> {
		handle<node> node = m_ast.create_node<bool_literal>(node_type::BOOL_LITERAL, 0);

		// since the current token is either BOOL_LITERAL_TRUE or BOOL_LITERAL_FALSE we can just do this:
		node->get<bool_literal>().value = m_tokens.get_current_token() == token_type::BOOL_LITERAL_TRUE;

		return node;
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
