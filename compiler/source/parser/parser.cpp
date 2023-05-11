#include "parser.h"

#include "codegen/abstract_syntax_tree/translation_unit_node.h"

// functions
#include "codegen/abstract_syntax_tree/functions/function_call_node.h"
#include "codegen/abstract_syntax_tree/functions/function_node.h"

// variables
#include "codegen/abstract_syntax_tree/variables/assignment_node.h"
#include "codegen/abstract_syntax_tree/variables/variable_access_node.h"
#include "codegen/abstract_syntax_tree/variables/array/array_allocation_node.h"
#include "codegen/abstract_syntax_tree/variables/array/array_access_node.h"
#include "codegen/abstract_syntax_tree/variables/array/array_assignment_node.h"
#include "codegen/abstract_syntax_tree/variables/declaration/local_declaration_node.h"
#include "codegen/abstract_syntax_tree/variables/declaration/global_declaration_node.h"
#include "codegen/abstract_syntax_tree/variables/variable_node.h"

// flow control
#include "codegen/abstract_syntax_tree/keywords/flow_control/return_node.h"
#include "codegen/abstract_syntax_tree/keywords/flow_control/if_else_node.h"
#include "codegen/abstract_syntax_tree/keywords/flow_control/while_node.h"
#include "codegen/abstract_syntax_tree/keywords/flow_control/for_node.h"
#include "codegen/abstract_syntax_tree/keywords/flow_control/break_node.h"

// types
#include "codegen/abstract_syntax_tree/keywords/types/numerical_literal_node.h"
#include "codegen/abstract_syntax_tree/keywords/types/char_node.h"
#include "codegen/abstract_syntax_tree/keywords/types/string_node.h"
#include "codegen/abstract_syntax_tree/keywords/types/bool_node.h"

// operators
// unary
// arithmetic
#include "codegen/abstract_syntax_tree/operators/unary/arithmetic/operator_post_decrement.h"
#include "codegen/abstract_syntax_tree/operators/unary/arithmetic/operator_post_increment.h"
#include "codegen/abstract_syntax_tree/operators/unary/arithmetic/operator_pre_decrement.h"
#include "codegen/abstract_syntax_tree/operators/unary/arithmetic/operator_pre_increment.h"

// binary
// arithmetic
#include "codegen/abstract_syntax_tree/operators/binary/arithmetic/operator_addition_node.h"
#include "codegen/abstract_syntax_tree/operators/binary/arithmetic/operator_division_node.h"
#include "codegen/abstract_syntax_tree/operators/binary/arithmetic/operator_multiplication_node.h"
#include "codegen/abstract_syntax_tree/operators/binary/arithmetic/operator_subtraction_node.h"
#include "codegen/abstract_syntax_tree/operators/binary/arithmetic/operator_modulo_node.h"
// logical
#include "codegen/abstract_syntax_tree/operators/binary/logical/operator_conjunction_node.h"
#include "codegen/abstract_syntax_tree/operators/binary/logical/operator_disjunction_node.h"
#include "codegen/abstract_syntax_tree/operators/binary/logical/operator_greater_than_node.h"
#include "codegen/abstract_syntax_tree/operators/binary/logical/operator_greater_than_equal_to_node.h"
#include "codegen/abstract_syntax_tree/operators/binary/logical/operator_less_than_node.h"
#include "codegen/abstract_syntax_tree/operators/binary/logical/operator_less_than_equal_to_node.h"
#include "codegen/abstract_syntax_tree/operators/binary/logical/operator_equals_node.h"
#include "codegen/abstract_syntax_tree/operators/binary/logical/operator_not_equals_node.h"
#include "codegen/abstract_syntax_tree/keywords/flow_control/break_node.h"

namespace channel {
	parser::parser(const lexer& lexer)
		: m_lexer(lexer) {}

	bool parser::parse() {
		std::vector<node*> nodes;

		while (true) {
			if (peek_next_token() == token::end_of_file) {
				m_abstract_syntax_tree.add_node(new translation_unit_node(nodes));
				return true;
			}

			node* node;

			if (peek_is_function_definition()) {
				// parse a top-level function definition
				if (!parse_function_definition(node)) {
					return false;
				}
			}
			else {
				// parse a global statement
				if (!parse_global_statement(node)) {
					return false;
				}
			}

			nodes.push_back(node);
		}
	}

	const abstract_syntax_tree& parser::get_abstract_syntax_tree() {
		return m_abstract_syntax_tree;
	}

	void parser::get_next_token() {
		m_current_token = m_lexer.get_token();
	}

	bool parser::expect_next_token(token token) {
		get_next_token();

		if (m_current_token.get_token() == token) {
			return true;
		}

		error::emit<3000>(m_current_token.get_token_position(), token, m_current_token.get_token()).print();
		return false; // return on failure
	}

	bool parser::parse_function_definition(node*& out_node) {
		type return_type;
		if (!parse_type(return_type)) {
			return false; // return on failure
		}

		const token_position position = m_current_token.get_token_position();
		get_next_token(); // identifier (guaranteed)
		const std::string identifier = m_current_token.get_value();
		get_next_token(); // l_parenthesis (guaranteed)

		std::vector<std::pair<std::string, type>> arguments;

		// parse arguments
		token next_token = peek_next_token();
		if (next_token != token::r_parenthesis) {
			while (true) {
				type argument_type;
				if (!parse_type(argument_type)) {
					return false; // return on failure
				}

				if (!expect_next_token(token::identifier)) {
					return false; // return on failure
				}

				std::string argument_name = m_current_token.get_value();
				arguments.emplace_back(argument_name, argument_type);

				// get_next_token(); // comma || type || other
				next_token = peek_next_token();
				if (next_token == token::comma) {
					get_next_token(); // comma (guaranteed)
				}
				else if (next_token != token::r_parenthesis) {
					error::emit<3000>(m_current_token.get_token_position(), token::r_parenthesis, m_current_token.get_token()).print();
					return false; // return on failure
				}
				else {
					get_next_token(); // r_parenthesis(guaranteed)
					break;
				}
			}
		}
		else {
			get_next_token(); // r_parenthesis (guaranteed)
		}

		std::vector<node*> statements;
		if (!parse_local_statements(statements)) {
			return false; // return on failure
		}

		out_node = new function_node(position, return_type, identifier, arguments, statements);
		return true;
	}

	bool parser::parse_global_statement(node*& out_node) {
		const token token = peek_next_token(); // identifier || type || keyword

		if (is_token_type(token)) {
			// statements beginning with a type keyword have to be variable declarations
			if (!parse_declaration(out_node, true)) {
				return false; // return on failure
			}
		}
		else {
			switch (token) {
			case token::identifier:
				// assignment statement
				if (!parse_assignment(out_node)) {
					return false; // return on failure
				}
				break;
			default:
				error::emit<3001>(m_current_token.get_token_position(), token).print();
				return false; // return on failure
			}
		}

		if (!expect_next_token(token::semicolon)) {
			return false; // return on failure
		}

		return true;
	}

	bool parser::parse_local_statements(std::vector<node*>& out_statements) {
		if (!expect_next_token(token::l_brace)) {
			return false;  // return on failure
		}

		bool met_block_break = false;
		token next_token = peek_next_token();

		// get all statements in the current scope
		while (next_token != token::r_brace) {
			node* statement;
			if (!parse_local_statement(statement)) {
				return false;  // return on failure
			}

			// check if we've met a block break token
			if (met_block_break == false) {
				met_block_break = is_token_block_break(next_token);
				// if we haven't, we can add the statement to the vector
				// this prevents us from adding unreachable nodes to the AST
				out_statements.push_back(statement);
			}

			next_token = peek_next_token();
		}

		get_next_token(); // r_brace (guaranteed)
		return true;
	}

	bool parser::parse_local_statement(node*& out_node) {
		const token next_token = peek_next_token(); // identifier || type || keyword

		if (is_token_type(next_token)) {
			// statements beginning with a type keyword have to be variable declarations
			if (!parse_declaration(out_node, false)) {
				return false;  // return on failure
			}
		}
		else {
			switch (next_token) {
			case token::identifier:
				if (!parse_local_statement_identifier(out_node)) {
					return false; // return on failure
				}
				break;
			case token::l_parenthesis:
				// parse a deep expression (parenthesized expression)
				if (!parse_deep_expression(out_node, type::unknown())) {
					return false; // return on failure
				}

				// check for post unary operators after deep expression
				if (peek_next_token() == token::operator_increment || peek_next_token() == token::operator_decrement) {
					node* operand = out_node;
					if (!parse_post_operator(operand, out_node)) {
						return false; // return on failure
					}
				}
				break;
			case token::operator_increment:
			case token::operator_decrement:
				// check if the next token is an identifier or an opening parenthesis
				token next_next_token;
				next_next_token = peek_nth_token(2);

				if (next_next_token == token::identifier || next_next_token == token::l_parenthesis) {
					if (!parse_pre_operator(out_node)) {
						return false; // return on failure
					}
				}
				else {
					error::emit<3004>(m_current_token.get_token_position()).print();
					return false; // return on failure
				}
				break;
			case token::keyword_return:
				if (!parse_return_statement(out_node)) {
					return false;
				}
				break;
			case token::keyword_break:
				if (!parse_break_keyword(out_node)) {
					return false; // return on failure
				}
				break;
			case token::keyword_if:
				// return right away since we don't want to check for a semicolon at the end of the statement
				return parse_if_else_statement(out_node);
			case token::keyword_while:
				// return right away since we don't want to check for a semicolon at the end of the statement
				return parse_while_loop(out_node);
			case token::keyword_for:
				// return right away since we don't want to check for a semicolon at the end of the statement
				return parse_for_loop(out_node);
			default:
				error::emit<3001>(m_current_token.get_token_position(), next_token).print();
				return false; // return on failure
			}
		}

		if (!expect_next_token(token::semicolon)) {
			return false; // return on failure
		}

		return true;
	}

	bool parser::parse_local_statement_identifier(node*& out_node) {
		if (peek_is_function_call()) {
			// function call statement
			if (!parse_function_call(out_node)) {
				return false; // return on failure
			}
		}
		else if (peek_is_array_index_access()) {
			// array assignment
			if (!parse_array_assignment(out_node)) {
				return false; // return on failure
			}
		}
		else if (peek_is_assignment()) {
			// assignment statement
			if (!parse_assignment(out_node)) {
				return false; // return on failure
			}
		}
		else {
			// create a simple access node
			get_next_token(); // identifier (guaranteed)
			out_node = new variable_access_node(m_current_token.get_token_position(), m_current_token.get_value());
		}

		// check for post unary operators after identifier, deep expression, or array index access
		if (peek_next_token() == token::operator_increment || peek_next_token() == token::operator_decrement) {
			node* operand = out_node;
			if (!parse_post_operator(operand, out_node)) {
				return false; // return on failure
			}
		}

		return true;
	}

	bool parser::parse_if_else_statement(node*& out_node) {
		std::vector<node*> conditions;
		std::vector<std::vector<node*>> branches;
		const token_position position = m_current_token.get_token_position();
		bool has_else = false;

		while (true) {
			if (!has_else && peek_next_token() == token::keyword_else) {
				get_next_token(); // keyword_else (guaranteed)
				if (peek_next_token() != token::keyword_if) {
					has_else = true;
				}
			}

			if (peek_next_token() == token::keyword_if || has_else) {
				if (!has_else) {
					get_next_token(); // keyword_if (guaranteed)
				}

				node* condition = nullptr;
				if (!has_else) {
					if (!expect_next_token(token::l_parenthesis) ||
						!parse_expression(condition) ||
						!expect_next_token(token::r_parenthesis)) {
						return false; // return on failure
					}
				}

				conditions.push_back(condition);
			}
			else {
				break;
			}

			std::vector<node*> branch_statements;
			if (!parse_local_statements(branch_statements)) {
				return false; // return on failure
			}

			branches.push_back(branch_statements);

			if (has_else) {
				break;
			}
		}

		out_node = new if_else_node(position, conditions, branches);
		return true;
	}

	bool parser::parse_while_loop(node*& out_node) {
		get_next_token(); // keyword_while (guaranteed)
		const token_position position = m_current_token.get_token_position();

		if (!expect_next_token(token::l_parenthesis)) {
			return false; // return on failure
		}

		node* loop_condition_node;
		if (!parse_expression(loop_condition_node)) {
			return false; // return on failure
		}

		if (!expect_next_token(token::r_parenthesis)) {
			return false; // return on failure
		}

		if (!expect_next_token(token::l_brace)) {
			return false; // return on failure
		}

		std::vector<node*> loop_statements;
		while (peek_next_token() != token::r_brace) {
			node* statement;
			if (!parse_local_statement(statement)) {
				return false; // return on failure
			}

			loop_statements.push_back(statement);
		}

		get_next_token(); // r_brace (guaranteed)
		out_node = new while_node(position, loop_condition_node, loop_statements);
		return true;
	}

	bool parser::parse_loop_increment(node*& out_node) {
		switch (const token next_token = peek_next_token()) {
		case token::identifier:
			if (!parse_local_statement_identifier(out_node)) {
				return false; // return on failure
			}
			break;
		case token::operator_increment:
		case token::operator_decrement:
			// check if the next token is an identifier or an opening parenthesis
			token next_next_token;
			next_next_token = peek_nth_token(2);

			if (next_next_token == token::identifier || next_next_token == token::l_parenthesis) {
				if (!parse_pre_operator(out_node)) {
					return false; // return on failure
				}
			}
			else {
				error::emit<3004>(m_current_token.get_token_position()).print();
				return false; // return on failure
			}
			break;
		case token::r_parenthesis:
			break;
		default:
			error::emit<3001>(m_current_token.get_token_position(), next_token).print();
			return false; // return on failure
		}

		return true;
	}

	bool parser::parse_for_loop(node*& out_node) {
		get_next_token(); // keyword_for (guaranteed)
		const token_position position = m_current_token.get_token_position();

		if (!expect_next_token(token::l_parenthesis)) {
			return false; // return on failure
		}

		// parse the initialization section
		node* loop_initialization_node;
		const token next_token = peek_next_token();

		if (is_token_type(next_token)) {
			// statements beginning with a type keyword have to be variable declarations
			if (!parse_declaration(loop_initialization_node, false)) {
				return false; // return on failure
			}
		}
		else {
			switch (next_token) {
			case token::identifier:
				if (!parse_local_statement_identifier(loop_initialization_node)) {
					return false; // return on failure
				}
				break;
			default:
				error::emit<3001>(m_current_token.get_token_position(), next_token).print();
				return false; // return on failure
			}
		}

		if (!expect_next_token(token::semicolon)) {
			return false;
		}

		// parse the loop condition section
		node* loop_condition_node;
		if (!parse_expression(loop_condition_node)) {
			return false; // return on failure
		}

		if (!expect_next_token(token::semicolon)) {
			return false; // return on failure
		}

		// parse the increment section
		std::vector<node*> post_iteration_nodes;

		while (true) {
			node* post_iteration_node;

			if (peek_next_token() == token::r_parenthesis) {
				break;
			}

			if (!parse_loop_increment(post_iteration_node)) {
				return false; // return on failure
			}

			post_iteration_nodes.push_back(post_iteration_node);

			if (peek_next_token() != token::comma) {
				break;
			}

			get_next_token(); // comma (guaranteed)
		}

		if (!expect_next_token(token::r_parenthesis)) {
			return false; // return on failure
		}

		if (!expect_next_token(token::l_brace)) {
			return false; // return on failure
		}

		// parse body statements
		std::vector<node*> loop_statements;
		while (peek_next_token() != token::r_brace) {
			node* statement;
			if (!parse_local_statement(statement)) {
				return false; // return on failure
			}

			loop_statements.push_back(statement);
		}

		get_next_token(); // r_brace (guaranteed)
		out_node = new for_node(position, loop_initialization_node, loop_condition_node, post_iteration_nodes, loop_statements);
		return true;
	}

	bool parser::parse_array_assignment(node*& out_node) {
		get_next_token(); // identifier (guaranteed)
		const std::string identifier = m_current_token.get_value();
		const token_position position = m_current_token.get_token_position();

		std::vector<node*> index_nodes;
		while (peek_next_token() == token::l_bracket) {
			get_next_token(); // l_bracket (guaranteed)
			node* index_node;
			if (!parse_expression(index_node, type(type::base::u64, 0))) {
				return false; // return on failure
			}

			index_nodes.push_back(index_node);

			// make sure the next token is a right square bracket
			if (!expect_next_token(token::r_bracket)) {
				return false; // return on failure
			}
		}

		const token next_token = peek_next_token();
		if (next_token == token::operator_assignment) {
			get_next_token();

			// parse access-assignment
			node* value;
			if (peek_is_function_call()) {
				if (!parse_function_call(value)) {
					return false; // return on failure
				}
			}
			else {
				if (!parse_expression(value)) {
					return false; // return on failure
				}
			}

			node* array_node = new variable_node(m_current_token.get_token_position(), identifier);
			out_node = new array_assignment_node(position, array_node, index_nodes, value);
		}
		else if (next_token == token::operator_increment || next_token == token::operator_decrement) {
			node* array_node = new variable_node(m_current_token.get_token_position(), identifier);
			out_node = new array_access_node(position, array_node, index_nodes);

			if (!parse_post_operator(out_node, out_node)) {
				return false; // return on failure
			}
		}

		return true;
	}

	bool parser::parse_assignment(node*& out_node) {
		get_next_token(); // identifier (guaranteed)
		node* variable = new variable_node(m_current_token.get_token_position(), m_current_token.get_value());

		if (!expect_next_token(token::operator_assignment)) {
			return false;  // return on failure
		}

		// parse access-assignment
		node* value;
		if (peek_is_function_call()) {
			if (!parse_function_call(value)) {
				return false; // return on failure
			}
		}
		else {
			if (!parse_expression(value)) {
				return false; // return on failure
			}
		}

		out_node = new assignment_node(m_current_token.get_token_position(), variable, value);
		return true;
	}

	bool parser::parse_array_access(node*& out_node) {
		get_next_token(); // identifier (guaranteed)
		const std::string identifier = m_current_token.get_value();

		node* array_node = new variable_node(m_current_token.get_token_position(), identifier);
		std::vector<node*> index_nodes;

		get_next_token(); // l_bracket (guaranteed)

		// parse access indices for multiple dimensions
		while (m_current_token.get_token() == token::l_bracket) {
			// parse access index
			node* array_index;
			if (!parse_expression(array_index, type(type::base::u64, 0))) {
				return false; // return on failure
			}
			index_nodes.push_back(array_index);

			if (!expect_next_token(token::r_bracket)) {
				return false; // return on failure
			}

			if (peek_next_token() != token::l_bracket) {
				break;
			}

			get_next_token(); // l_bracket(guaranteed)
		}

		out_node = new array_access_node(m_current_token.get_token_position(), array_node, index_nodes);
		return true;
	}

	bool parser::parse_function_call(node*& out_node) {
		get_next_token(); // identifier (guaranteed)
		const std::string identifier = m_current_token.get_value();
		get_next_token(); // l_parenthesis (guaranteed)
		std::vector<node*> arguments;

		token next_token = peek_next_token();
		if (next_token != token::r_parenthesis) {
			while (true) {
				node* argument;
				if (!parse_expression(argument)) {
					return false; // return on failure
				}

				arguments.push_back(argument);

				next_token = peek_next_token(); // comma || r_parenthesis || other

				if (next_token == token::comma) {
					get_next_token(); // comma (guaranteed)
				}
				else if (next_token == token::r_parenthesis) {
					break;
				}
				else {
					error::emit<3001>(m_current_token.get_token_position(), m_current_token.get_token()).print();
					return false;  // return on failure
				}
			}
		}

		if (!expect_next_token(token::r_parenthesis)) {
			return false; // return on failure
		}

		out_node = new function_call_node(m_current_token.get_token_position(), identifier, arguments);
		return true;
	}

	bool parser::parse_return_statement(node*& out_node) {
		const token_position position = m_current_token.get_token_position();
		get_next_token(); // keyword_return (guaranteed)

		node* expression;
		if (!parse_expression(expression)) {
			return false; // return on failure
		}

		out_node = new return_node(position, expression);
		return true;
	}

	bool parser::parse_declaration(node*& out_node, bool is_global) {
		const token_position position = m_current_token.get_token_position();

		type declaration_type;
		if (!parse_type(declaration_type)) {
			return false; // return on failure
		}

		if (!expect_next_token(token::identifier)) {
			return false; // return on failure
		}

		const std::string identifier = m_current_token.get_value();

		node* value = nullptr;
		if (peek_next_token() == token::operator_assignment) {
			get_next_token(); // operator_assignment
			if (!parse_expression(value, declaration_type)) {
				return false; // return on failure
			}
		}

		if (is_global) {
			out_node = new global_declaration_node(position, declaration_type, identifier, value);
			return true;
		}

		out_node = new local_declaration_node(position, declaration_type, identifier, value);
		return true;
	}

	bool parser::parse_expression(node*& out_node, type expression_type) {
		return parse_logical_conjunction(out_node, expression_type);
	}

	bool parser::parse_logical_conjunction(node*& out_node, type expression_type) {
		node* left;
		if (!parse_logical_disjunction(left, expression_type)) {
			return false;  // return on failure
		}

		while (peek_next_token() == token::operator_logical_conjunction) {
			get_next_token();
			const token_data op = m_current_token;

			node* right;
			if (!parse_logical_disjunction(right, expression_type)) {
				return false; // return on failure
			}

			left = new operator_conjunction_node(op.get_token_position(), left, right);
		}

		out_node = left;
		return true;
	}

	bool parser::parse_logical_disjunction(node*& out_node, type expression_type) {
		node* left;
		if (!parse_comparison(left, expression_type)) {
			return false; // return on failure
		}

		while (peek_next_token() == token::operator_logical_disjunction) {
			get_next_token();
			const token_data op = m_current_token;

			node* right;
			if (!parse_comparison(right, expression_type)) {
				return false; // return on failure
			}

			left = new operator_disjunction_node(op.get_token_position(), left, right);
		}

		out_node = left;
		return true;
	}

	bool parser::parse_comparison(node*& out_node, type expression_type) {
		node* left;
		if (!parse_term(left, expression_type)) {
			return false; // return on failure
		}

		while (
			peek_next_token() == token::operator_greater_than ||
			peek_next_token() == token::operator_greater_than_equal_to ||
			peek_next_token() == token::operator_less_than ||
			peek_next_token() == token::operator_less_than_equal_to ||
			peek_next_token() == token::operator_equals ||
			peek_next_token() == token::operator_not_equals) {
			get_next_token();
			const token_data op = m_current_token;

			node* right;
			if (!parse_term(right, expression_type)) {
				return false; // return on failure
			}

			switch (op.get_token()) {
			case token::operator_greater_than:
				left = new operator_greater_than_node(m_current_token.get_token_position(), left, right);
				break;
			case token::operator_greater_than_equal_to:
				left = new operator_greater_than_equal_to_node(m_current_token.get_token_position(), left, right);
				break;
			case token::operator_less_than:
				left = new operator_less_than_node(m_current_token.get_token_position(), left, right);
				break;
			case token::operator_less_than_equal_to:
				left = new operator_less_than_equal_to_node(m_current_token.get_token_position(), left, right);
				break;
			case token::operator_equals:
				left = new operator_equals_node(m_current_token.get_token_position(), left, right);
				break;
			case token::operator_not_equals:
				left = new operator_not_equals_node(m_current_token.get_token_position(), left, right);
				break;
			}
		}

		out_node = left;
		return true;
	}

	bool parser::parse_term(node*& out_node, type expression_type) {
		node* left;
		if (!parse_factor(left, expression_type)) {
			return false; // return on failure
		}

		while (
			peek_next_token() == token::operator_addition ||
			peek_next_token() == token::operator_subtraction) {
			get_next_token();
			const token_data op = m_current_token;

			node* right;
			if (!parse_factor(right, expression_type)) {
				return false; // return on failure
			}

			switch (op.get_token()) {
			case token::operator_addition:
				left = new operator_addition_node(op.get_token_position(), left, right);
				break;
			case token::operator_subtraction:
				left = new operator_subtraction_node(op.get_token_position(), left, right);
				break;
			}
		}

		out_node = left;
		return true;
	}

	bool parser::parse_factor(node*& out_node, type expression_type) {
		node* left;
		if (!parse_primary(left, expression_type)) {
			return false; // return on failure
		}

		while (
			peek_next_token() == token::operator_multiplication ||
			peek_next_token() == token::operator_division ||
			peek_next_token() == token::operator_modulo) {

			get_next_token();
			const token_data op = m_current_token;

			node* right;
			if (!parse_primary(right, expression_type)) {
				return false; // return on failure
			}

			switch (op.get_token()) {
			case token::operator_multiplication:
				left = new operator_multiplication_node(op.get_token_position(), left, right);
				break;
			case token::operator_division:
				left = new operator_division_node(op.get_token_position(), left, right);
				break;
			case token::operator_modulo:
				left = new operator_modulo_node(op.get_token_position(), left, right);
				break;
			}
		}

		out_node = left;
		return true;
	}

	bool parser::parse_primary(node*& out_node, type expression_type) {
		const token next_token = peek_next_token();

		if (is_token_numerical(next_token)) {
			// parse a number
			return parse_number(out_node, expression_type);
		}

		switch (next_token) {
		case token::operator_subtraction:
			return parse_negative_number(out_node, expression_type);
		case token::operator_increment:
		case token::operator_decrement:
			// check if the next token is an identifier or an opening parenthesis
			token next_next_token;
			next_next_token = peek_nth_token(2);

			if (next_next_token == token::identifier || next_next_token == token::l_parenthesis) {
				return parse_pre_operator(out_node);
			}

			error::emit<3004>(m_current_token.get_token_position()).print();
			return false; // return on failure
		case token::identifier:
			// parse a function call or an assignment
			return parse_primary_identifier(out_node);
		case token::l_parenthesis:
			// parse a deep expression
			return parse_deep_expression(out_node, expression_type);
		case token::keyword_new:
			// parse an allocation
			return parse_new_allocation(out_node);
		case token::char_literal:
			// parse a char literal
			return parse_char(out_node);
		case token::string_literal:
			// parse a string literal
			return parse_string(out_node);
		case token::bool_literal_true:
		case token::bool_literal_false:
			// parse a boolean
			return parse_bool(out_node);
		}

		error::emit<3001>(m_current_token.get_token_position(), m_current_token.get_token()).print();
		return false; // return on failure
	}

	bool parser::parse_number(node*& out_node, type expression_type) {
		get_next_token(); // type
		const std::string str_value = m_current_token.get_value();
		const type ty = expression_type.is_unknown() ? type(m_current_token.get_token(), 0) : expression_type;
		out_node = new numerical_literal_node(m_current_token.get_token_position(), str_value, ty);
		return true;

		// get_next_token(); // type
		// const std::string str_value = m_current_token.get_value();
		// const type ty = expression_type.is_unknown() ? type(m_current_token.get_token(), 0) : expression_type;
		// 
		// switch (ty.get_base()) {
		// 	// signed
		// case type::base::i8:  out_node = new i8_node(m_current_token.get_token_position(), std::stoll(str_value)); return true;
		// case type::base::i16: out_node = new i16_node(m_current_token.get_token_position(), std::stoll(str_value)); return true;
		// case type::base::i32: out_node = new i32_node(m_current_token.get_token_position(), std::stoll(str_value)); return true;
		// case type::base::i64: out_node = new i64_node(m_current_token.get_token_position(), std::stoll(str_value)); return true;
		// 	// unsigned
		// case type::base::u8:  out_node = new u8_node(m_current_token.get_token_position(), std::stoull(str_value)); return true;
		// case type::base::u16: out_node = new u16_node(m_current_token.get_token_position(), std::stoull(str_value)); return true;
		// case type::base::u32: out_node = new u32_node(m_current_token.get_token_position(), std::stoull(str_value)); return true;
		// case type::base::u64: out_node = new u64_node(m_current_token.get_token_position(), std::stoull(str_value)); return true;
		// 	// floating point
		// case type::base::f32: out_node = new f32_node(m_current_token.get_token_position(), std::stof(str_value)); return true;
		// case type::base::f64: out_node = new f64_node(m_current_token.get_token_position(), std::stod(str_value)); return true;
		// 	// bool
		// case type::base::boolean: out_node = new bool_node(m_current_token.get_token_position(), std::stoi(str_value)); return true;
		// default:
		// 	error::emit<3002>(m_current_token.get_token_position(), ty).print();
		// 	return false; // return on failure
		// }
	}

	bool parser::parse_char(node*& out_node) {
		get_next_token(); // char_literal (guaranteed)
		out_node = new char_node(m_current_token.get_token_position(), m_current_token.get_value()[0]);
		return true;
	}

	bool parser::parse_string(node*& out_node) {
		get_next_token(); // string_literal (guaranteed)
		out_node = new string_node(m_current_token.get_token_position(), m_current_token.get_value());
		return true;
	}

	bool parser::parse_bool(node*& out_node) {
		get_next_token(); // bool_literal_true || bool_literal_false (guaranteed)
		out_node = new bool_node(m_current_token.get_token_position(), m_current_token.get_token() == token::bool_literal_true);
		return true;
	}

	bool parser::parse_break_keyword(node*& out_node) {
		get_next_token(); // keyword_break (guaranteed)
		out_node = new break_node(m_current_token.get_token_position());
		return true;
	}

	bool parser::parse_post_operator(node* operand, node*& out_node) {
		get_next_token();

		if (m_current_token.get_token() == token::operator_increment) {
			out_node = new operator_post_increment(m_current_token.get_token_position(), operand);
		}
		else {
			out_node = new operator_post_decrement(m_current_token.get_token_position(), operand);
		}

		return true;
	}

	bool parser::parse_pre_operator(node*& out_node) {
		get_next_token();
		const token_data op = m_current_token;
		node* operand;

		if (!parse_primary(operand, type::unknown())) {
			return false; // return on failure
		}

		if (op.get_token() == token::operator_increment) {
			out_node = new operator_pre_increment(op.get_token_position(), operand);
		}
		else {
			out_node = new operator_pre_decrement(op.get_token_position(), operand);
		}

		return true;
	}

	bool parser::parse_negative_number(node*& out_node, type expression_type) {
		get_next_token(); // operator_subtraction (guaranteed)

		// negate the number by subtracting it from 0
		node* zero_node = create_zero_node(expression_type);
		node* number;

		if (!parse_number(number, expression_type)) {
			return false; // return on failure
		}

		out_node = new operator_subtraction_node(m_current_token.get_token_position(), zero_node, number);
		return true;
	}

	bool parser::parse_new_allocation(node*& out_node) {
		get_next_token(); // keyword_new (guaranteed)
		const token_position position = m_current_token.get_token_position();

		type allocation_type;
		if (!parse_type(allocation_type)) {
			return false; // return on failure
		}

		// l_bracket
		if (!expect_next_token(token::l_bracket)) {
			return false; // return on failure
		}

		// parse array size
		node* array_size;
		if (!parse_expression(array_size, type(type::base::u64, 0))) {
			return false; // return on failure
		}

		// r_bracket
		if (!expect_next_token(token::r_bracket)) {
			return false; // return on failure
		}

		out_node = new array_allocation_node(position, allocation_type, array_size);
		return true;
	}

	bool parser::parse_primary_identifier(node*& out_node) {
		if (peek_is_function_call()) {
			// parse a function call
			return parse_function_call(out_node);
		}

		if (peek_is_array_index_access()) {
			if (!parse_array_access(out_node)) {
				return false; // return on failure
			}
		}
		else {
			// parse an assignment
			get_next_token();
			const std::string identifier = m_current_token.get_value();
			out_node = new variable_access_node(m_current_token.get_token_position(), identifier);
		}

		// post increment
		const token next_token = peek_next_token();
		if (next_token == token::operator_increment || next_token == token::operator_decrement) {
			return parse_post_operator(out_node, out_node);
		}

		return true;
	}

	bool parser::parse_deep_expression(node*& out_node, type expression_type) {
		get_next_token(); // l_parenthesis (guaranteed)

		// nested expression
		if (!parse_expression(out_node, expression_type)) {
			return false; // return on failure
		}

		// r_parenthesis
		if (!expect_next_token(token::r_parenthesis)) {
			return false; // return on failure
		}

		return true;
	}

	bool parser::peek_is_function_definition() {
		// type
		if (!is_token_type(m_lexer.peek_token().get_token())) {
			m_lexer.synchronize_indices();
			return false; // return on failure
		}

		// pointers?
		token tok = m_lexer.peek_token().get_token();
		while (tok == token::operator_multiplication) {
			tok = m_lexer.peek_token().get_token();
		}

		// identifier
		if (tok != token::identifier) {
			m_lexer.synchronize_indices();
			return false; // return on failure
		}

		const bool result = m_lexer.peek_token().get_token() == token::l_parenthesis;
		m_lexer.synchronize_indices();
		return result;
	}

	bool parser::peek_is_function_call() {
		// identifier
		if (m_lexer.peek_token().get_token() != token::identifier) {
			m_lexer.synchronize_indices();
			return false; // return on failure
		}

		const bool result = m_lexer.peek_token().get_token() == token::l_parenthesis;
		m_lexer.synchronize_indices();
		return result;
	}

	bool parser::peek_is_assignment() {
		// identifier
		if (m_lexer.peek_token().get_token() != token::identifier) {
			m_lexer.synchronize_indices();
			return false; // return on failure
		}

		const bool result = m_lexer.peek_token().get_token() == token::operator_assignment;
		m_lexer.synchronize_indices();
		return result;
	}

	bool parser::peek_is_array_index_access() {
		// identifier
		if (m_lexer.peek_token().get_token() != token::identifier) {
			m_lexer.synchronize_indices();
			return false; // return on failure
		}

		const bool result = m_lexer.peek_token().get_token() == token::l_bracket;
		m_lexer.synchronize_indices();
		return result;
	}

	bool parser::peek_is_post_operator() {
		// identifier
		if (m_lexer.peek_token().get_token() != token::identifier) {
			m_lexer.synchronize_indices();
			return false; // return on failure
		}

		const token tok = m_lexer.peek_token().get_token();
		const bool result = tok == token::operator_decrement || tok == token::operator_increment;
		m_lexer.synchronize_indices();
		return result;
	}

	token parser::peek_next_token() {
		const token_data pair = m_lexer.peek_token();
		m_lexer.synchronize_indices();
		return pair.get_token();
	}

	token parser::peek_nth_token(u64 offset) {
		for (u64 i = 0; i < offset - 1; ++i) {
			m_lexer.peek_token();
		}

		const token_data pair = m_lexer.peek_token();
		m_lexer.synchronize_indices();
		return pair.get_token();
	}

	node* parser::create_zero_node(type expression_type) const {
		return new numerical_literal_node(m_current_token.get_token_position(), "0", expression_type);
		//switch (expression_type.get_base()) {
		//case type::base::i8:  return new i8_node(m_current_token.get_token_position(), 0);
		//case type::base::i16: return new i16_node(m_current_token.get_token_position(), 0);
		//case type::base::i32: return new i32_node(m_current_token.get_token_position(), 0);
		//case type::base::i64: return new i64_node(m_current_token.get_token_position(), 0);
		//case type::base::u8:  return new u8_node(m_current_token.get_token_position(), 0);
		//case type::base::u16: return new u16_node(m_current_token.get_token_position(), 0);
		//case type::base::u32: return new u32_node(m_current_token.get_token_position(), 0);
		//case type::base::u64: return new u64_node(m_current_token.get_token_position(), 0);
		//case type::base::f32: return new f32_node(m_current_token.get_token_position(), 0.0f);
		//case type::base::f64: return new f64_node(m_current_token.get_token_position(), 0.0);
		//default:
		//	ASSERT(false, "[parser]: cannot convert '" + expression_type.to_string() + "' to a type keyword");
		//	return nullptr;
		//}
	}

	bool parser::parse_type(type& ty) {
		get_next_token();

		if (!is_token_type(m_current_token.get_token())) {
			error::emit<3003>(m_current_token.get_token_position(), m_current_token.get_token()).print();
			return false; // return on failure
		}

		ty = type(m_current_token.get_token(), 0);

		// check if the next token is an asterisk
		while (peek_next_token() == token::operator_multiplication) {
			get_next_token(); // operator_multiplication (guaranteed)
			ty.set_pointer_level(ty.get_pointer_level() + 1);
		}

		return true;
	}
}
