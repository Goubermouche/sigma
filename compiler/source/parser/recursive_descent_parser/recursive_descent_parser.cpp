#include "recursive_descent_parser.h"

// functions
#include "code_generator/abstract_syntax_tree/functions/function_call_node.h"
#include "code_generator/abstract_syntax_tree/functions/function_node.h"

// keywords
#include "code_generator/abstract_syntax_tree/keywords/file_include_node.h"

// variables
#include "code_generator/abstract_syntax_tree/variables/assignment_node.h"
#include "code_generator/abstract_syntax_tree/variables/variable_access_node.h"
#include "code_generator/abstract_syntax_tree/variables/array/array_allocation_node.h"
#include "code_generator/abstract_syntax_tree/variables/array/array_access_node.h"
#include "code_generator/abstract_syntax_tree/variables/array/array_assignment_node.h"
#include "code_generator/abstract_syntax_tree/variables/declaration/local_declaration_node.h"
#include "code_generator/abstract_syntax_tree/variables/declaration/global_declaration_node.h"
#include "code_generator/abstract_syntax_tree/variables/variable_node.h"

// flow control
#include "code_generator/abstract_syntax_tree/keywords/flow_control/return_node.h"
#include "code_generator/abstract_syntax_tree/keywords/flow_control/if_else_node.h"
#include "code_generator/abstract_syntax_tree/keywords/flow_control/while_node.h"
#include "code_generator/abstract_syntax_tree/keywords/flow_control/for_node.h"
#include "code_generator/abstract_syntax_tree/keywords/flow_control/break_node.h"

// types
#include "code_generator/abstract_syntax_tree/keywords/types/numerical_literal_node.h"
#include "code_generator/abstract_syntax_tree/keywords/types/char_node.h"
#include "code_generator/abstract_syntax_tree/keywords/types/string_node.h"
#include "code_generator/abstract_syntax_tree/keywords/types/bool_node.h"

// operators
// unary
// arithmetic
#include "code_generator/abstract_syntax_tree/operators/unary/arithmetic/operator_post_decrement_node.h"
#include "code_generator/abstract_syntax_tree/operators/unary/arithmetic/operator_post_increment_node.h"
#include "code_generator/abstract_syntax_tree/operators/unary/arithmetic/operator_pre_decrement_node.h"
#include "code_generator/abstract_syntax_tree/operators/unary/arithmetic/operator_pre_increment_node.h"
// bitwise
#include "code_generator/abstract_syntax_tree/operators/unary/bitwise/operator_bitwise_not_node.h"
// logical
#include "code_generator/abstract_syntax_tree/operators/unary/logical/operator_not_node.h"

// binary
// arithmetic
#include "code_generator/abstract_syntax_tree/operators/binary/arithmetic/operator_addition_assignment_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/arithmetic/operator_addition_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/arithmetic/operator_subtraction_assignment_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/arithmetic/operator_subtraction_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/arithmetic/operator_multiplication_assignment_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/arithmetic/operator_multiplication_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/arithmetic/operator_division_assignment_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/arithmetic/operator_division_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/arithmetic/operator_modulo_assignment_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/arithmetic/operator_modulo_node.h"
// bitwise
#include "code_generator/abstract_syntax_tree/operators/binary/bitwise/operator_bitwise_and_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/bitwise/operator_bitwise_left_shift_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/bitwise/operator_bitwise_or_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/bitwise/operator_bitwise_right_shift_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/bitwise/operator_bitwise_xor_node.h"
// logical
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_conjunction_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_disjunction_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_greater_than_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_greater_than_equal_to_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_less_than_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_less_than_equal_to_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_equals_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_not_equals_node.h"
#include "code_generator/abstract_syntax_tree/keywords/flow_control/break_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/arithmetic/operator_addition_assignment_node.h"

namespace sigma {
	recursive_descent_parser::recursive_descent_parser() {}

	outcome::result<void> recursive_descent_parser::parse() {
		while (true) {
			if (peek_next_token() == token::end_of_file) {
				return outcome::success();
			}

			node* node;

			if (peek_is_function_definition()) {
				// parse a top-level function definition
				OUTCOME_TRY(parse_function_definition(node));
			}
			else if(peek_is_file_include()) {
				OUTCOME_TRY(parse_file_include(node));
			}
			else {
				// parse a global statement
				OUTCOME_TRY(parse_global_statement(node));
			}

			m_abstract_syntax_tree->add_node(node);
		}
	}

	void recursive_descent_parser::get_next_token() {
		m_current_token = m_token_list.get_token();
	}

	outcome::result<void> recursive_descent_parser::expect_next_token(token token) {
		get_next_token();

		if (m_current_token.get_token() == token) {
			return outcome::success();
		}

		return outcome::failure(error::emit<3000>(
			m_current_token.get_token_location(),
			token,
			m_current_token.get_token()
		));
	}

	outcome::result<void> recursive_descent_parser::parse_function_definition(node*& out_node) {
		type return_type;
		OUTCOME_TRY(parse_type(return_type));

		const file_position location = m_current_token.get_token_location();
		get_next_token(); // identifier (guaranteed)
		const std::string identifier = m_current_token.get_value();
		get_next_token(); // l_parenthesis (guaranteed)

		std::vector<std::pair<std::string, type>> arguments;

		// parse arguments
		token next_token = peek_next_token();
		if (next_token != token::r_parenthesis) {
			while (true) {
				type argument_type;
				OUTCOME_TRY(parse_type(argument_type));
				OUTCOME_TRY(expect_next_token(token::identifier));

				std::string argument_name = m_current_token.get_value();
				arguments.emplace_back(argument_name, argument_type);

				// get_next_token(); // comma || type || other
				next_token = peek_next_token();
				if (next_token == token::comma) {
					get_next_token(); // comma (guaranteed)
				}
				else if (next_token != token::r_parenthesis) {
					return outcome::failure(error::emit<3000>(
						m_current_token.get_token_location(), 
						token::r_parenthesis, 
						m_current_token.get_token()
					)); // return on failure
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
		OUTCOME_TRY(parse_local_statements(statements));

		out_node = new function_node(
			location,
			return_type, 
			false, 
			identifier, 
			arguments, 
			statements
		);

		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_file_include(node*& out_node) {
		// #include string literal
		OUTCOME_TRY(expect_next_token(token::hash));
		OUTCOME_TRY(expect_next_token(token::keyword_include));
		OUTCOME_TRY(expect_next_token(token::string_literal));

		const token_data file_token = m_current_token;
		out_node = new file_include_node(m_current_token.get_token_location(), m_current_token.get_value());
		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_global_statement(node*& out_node) {
		const token token = peek_next_token(); // identifier || type || keyword

		if (is_token_type(token)) {
			// statements beginning with a type keyword have to be variable declarations
			OUTCOME_TRY(parse_declaration(out_node, true));
		}
		else {
			switch (token) {
			case token::identifier:
				// assignment statement
				OUTCOME_TRY(parse_assignment(out_node));
				break;
			default:
				return outcome::failure(error::emit<3001>(
					m_current_token.get_token_location(),
					token
				));
			}
		}

		OUTCOME_TRY(expect_next_token(token::semicolon));
		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_local_statements(std::vector<node*>& out_statements) {
		OUTCOME_TRY(expect_next_token(token::l_brace));

		bool met_block_break = false;
		token next_token = peek_next_token();

		// get all statements in the current scope
		while (next_token != token::r_brace) {
			node* statement;
			OUTCOME_TRY(parse_local_statement(statement));

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
		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_local_statement(node*& out_node) {
		const token next_token = peek_next_token(); // identifier || type || keyword

		if (is_token_type(next_token)) {
			// statements beginning with a type keyword have to be variable declarations
			OUTCOME_TRY(parse_declaration(out_node, false));
		}
		else {
			switch (next_token) {
			case token::identifier:
				OUTCOME_TRY(parse_local_statement_identifier(out_node));
				break;
			case token::l_parenthesis:
				// parse a deep expression (parenthesized expression)
				OUTCOME_TRY(parse_deep_expression(out_node, type::unknown()));

				// check for post unary operators after deep expression
				if (peek_next_token() == token::operator_increment || peek_next_token() == token::operator_decrement) {
					node* operand = out_node;
					OUTCOME_TRY(parse_post_operator(operand, out_node));
				}
				break;
			case token::operator_increment:
			case token::operator_decrement:
			case token::operator_not:
			case token::operator_bitwise_not:
				// check if the next token is an identifier or an opening parenthesis
				token next_next_token;
				next_next_token = peek_nth_token(2);

				if (next_next_token == token::identifier || next_next_token == token::l_parenthesis) {
					OUTCOME_TRY(parse_pre_operator(out_node));
				}
				else {
					return outcome::failure(error::emit<3004>(
						m_current_token.get_token_location()
					)); // return on failure
				}
				break;
			case token::keyword_return:
				OUTCOME_TRY(parse_return_statement(out_node));
				break;
			case token::keyword_break:
				OUTCOME_TRY(parse_break_keyword(out_node));
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
				return outcome::failure(error::emit<3001>(
					m_current_token.get_token_location(),
					next_token
				)); // return on failure
			}
		}

		OUTCOME_TRY(expect_next_token(token::semicolon));
		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_local_statement_identifier(node*& out_node) {
		if (peek_is_function_call()) {
			// function call statement
			OUTCOME_TRY(parse_function_call(out_node));
		}
		else if (peek_is_array_index_access()) {
			// array assignment
			OUTCOME_TRY(parse_array_assignment(out_node));
		}
		else if (peek_is_assignment()) {
			// assignment statement
			OUTCOME_TRY(parse_assignment(out_node));
		}
		else {
			// create a simple access node
			get_next_token(); // identifier (guaranteed)
			out_node = new variable_access_node(m_current_token.get_token_location(), m_current_token.get_value());
		}

		// check for post unary operators after identifier, deep expression, or array index access
		const token next_token = peek_next_token();
		if (next_token == token::operator_increment || next_token == token::operator_decrement) {
			return parse_post_operator(out_node, out_node);
		}
		// compound operation
		if (is_token_compound_op(next_token)) {
			return parse_compound_operation(out_node, out_node);
		}

		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_if_else_statement(node*& out_node) {
		std::vector<node*> conditions;
		std::vector<std::vector<node*>> branches;
		const file_position location = m_current_token.get_token_location();
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
					OUTCOME_TRY(expect_next_token(token::l_parenthesis));
					OUTCOME_TRY(parse_expression(condition));
					OUTCOME_TRY(expect_next_token(token::r_parenthesis));
				}

				conditions.push_back(condition);
			}
			else {
				break;
			}

			std::vector<node*> branch_statements;
			OUTCOME_TRY(parse_local_statements(branch_statements));

			branches.push_back(branch_statements);

			if (has_else) {
				break;
			}
		}

		out_node = new if_else_node(location, conditions, branches);
		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_while_loop(node*& out_node) {
		get_next_token(); // keyword_while (guaranteed)
		const file_position location = m_current_token.get_token_location();

		OUTCOME_TRY(expect_next_token(token::l_parenthesis));

		node* loop_condition_node;
		OUTCOME_TRY(parse_expression(loop_condition_node));
		OUTCOME_TRY(expect_next_token(token::r_parenthesis));
		OUTCOME_TRY(expect_next_token(token::l_brace));

		std::vector<node*> loop_statements;
		while (peek_next_token() != token::r_brace) {
			node* statement;
			OUTCOME_TRY(parse_local_statement(statement));

			loop_statements.push_back(statement);
		}

		get_next_token(); // r_brace (guaranteed)
		out_node = new while_node(location, loop_condition_node, loop_statements);
		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_loop_increment(node*& out_node) {
		switch (const token next_token = peek_next_token()) {
		case token::identifier:
			OUTCOME_TRY(parse_local_statement_identifier(out_node));
			break;
		case token::operator_increment:
		case token::operator_decrement:
		case token::operator_not:
		case token::operator_bitwise_not:
			// check if the next token is an identifier or an opening parenthesis
			token next_next_token;
			next_next_token = peek_nth_token(2);

			if (
				next_next_token == token::identifier || 
				next_next_token == token::l_parenthesis) {
				OUTCOME_TRY(parse_pre_operator(out_node));
			}
			else {
				return outcome::failure(error::emit<3004>(
					m_current_token.get_token_location()
				)); // return on failure
			}
			break;
		case token::r_parenthesis:
			break;
		default:
			return outcome::failure(error::emit<3001>(
				m_current_token.get_token_location(), 
				next_token
			)); // return on failure
		}

		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_for_loop(node*& out_node) {
		get_next_token(); // keyword_for (guaranteed)
		const file_position location = m_current_token.get_token_location();

		OUTCOME_TRY(expect_next_token(token::l_parenthesis));

		// parse the initialization section
		node* loop_initialization_node;
		const token next_token = peek_next_token();

		if (is_token_type(next_token)) {
			// statements beginning with a type keyword have to be variable declarations
			OUTCOME_TRY(parse_declaration(loop_initialization_node, false));
		}
		else {
			switch (next_token) {
			case token::identifier:
				OUTCOME_TRY(parse_local_statement_identifier(loop_initialization_node));
				break;
			default:
				return outcome::failure(error::emit<3001>(
					m_current_token.get_token_location(), 
					next_token
				)); // return on failure
			}
		}

		OUTCOME_TRY(expect_next_token(token::semicolon));

		// parse the loop condition section
		node* loop_condition_node;
		OUTCOME_TRY(parse_expression(loop_condition_node));
		OUTCOME_TRY(expect_next_token(token::semicolon));

		// parse the increment section
		std::vector<node*> post_iteration_nodes;

		while (true) {
			node* post_iteration_node;

			if (peek_next_token() == token::r_parenthesis) {
				break;
			}

			OUTCOME_TRY(parse_loop_increment(post_iteration_node));
			post_iteration_nodes.push_back(post_iteration_node);

			if (peek_next_token() != token::comma) {
				break;
			}

			get_next_token(); // comma (guaranteed)
		}

		OUTCOME_TRY(expect_next_token(token::r_parenthesis));
		OUTCOME_TRY(expect_next_token(token::l_brace));

		// parse body statements
		std::vector<node*> loop_statements;
		while (peek_next_token() != token::r_brace) {
			node* statement;
			OUTCOME_TRY(parse_local_statement(statement));
			loop_statements.push_back(statement);
		}

		get_next_token(); // r_brace (guaranteed)
		out_node = new for_node(
			location,
			loop_initialization_node, 
			loop_condition_node,
			post_iteration_nodes, 
			loop_statements
		);

		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_compound_operation(node*& out_node, node* left_operand) {
		// operator_addition_assignment ||
		// operator_subtraction_assignment || 
		// operator_multiplication_assignment || 
		// operator_modulo_assignment || 
		// operator_division_assignment (guaranteed)
		get_next_token();
		const token op = m_current_token.get_token();

		// rhs of the expression
		node* expression;
		OUTCOME_TRY(parse_expression(expression));

		switch (op) {
		case token::operator_addition_assignment:
			out_node = new operator_addition_assignment_node(
				m_current_token.get_token_location(),
				left_operand,
				expression
			);
			break;
		case token::operator_subtraction_assignment:
			out_node = new operator_subtraction_assignment_node(
				m_current_token.get_token_location(),
				left_operand,
				expression
			);
			break;
		case token::operator_multiplication_assignment:
			out_node = new operator_multiplication_assignment_node(
				m_current_token.get_token_location(),
				left_operand,
				expression
			);
			break;
		case token::operator_division_assignment:
			out_node = new operator_division_assignment_node(
				m_current_token.get_token_location(), 
				left_operand, 
				expression
			);
			break;
		case token::operator_modulo_assignment:
			out_node = new operator_modulo_assignment_node(
				m_current_token.get_token_location(), 
				left_operand,
				expression
			);
			break;
		}

		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_array_assignment(node*& out_node) {
		get_next_token(); // identifier (guaranteed)
		const std::string identifier = m_current_token.get_value();
		const file_position location = m_current_token.get_token_location();

		std::vector<node*> index_nodes;
		while (peek_next_token() == token::l_bracket) {
			get_next_token(); // l_bracket (guaranteed)
			node* index_node;
			OUTCOME_TRY(parse_expression(index_node, type(type::base::u64, 0)));

			index_nodes.push_back(index_node);

			// make sure the next token is a right square bracket
			OUTCOME_TRY(expect_next_token(token::r_bracket));
		}

		const token next_token = peek_next_token();
		if (next_token == token::operator_assignment) {
			get_next_token();

			// parse access-assignment
			node* value;
			if (peek_is_function_call()) {
				OUTCOME_TRY(parse_function_call(value));
			}
			else {
				OUTCOME_TRY(parse_expression(value));
			}

			node* array_node = new variable_node(m_current_token.get_token_location(), identifier);
			out_node = new array_assignment_node(location, array_node, index_nodes, value);
		}
		else if (next_token == token::operator_increment || next_token == token::operator_decrement) {
			node* array_node = new variable_node(m_current_token.get_token_location(), identifier);
			out_node = new array_access_node(location, array_node, index_nodes);

			OUTCOME_TRY(parse_post_operator(out_node, out_node));
		}

		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_assignment(node*& out_node) {
		get_next_token(); // identifier (guaranteed)
		node* variable = new variable_node(m_current_token.get_token_location(), m_current_token.get_value());

		OUTCOME_TRY(expect_next_token(token::operator_assignment));

		// parse access-assignment
		node* value;
		if (peek_is_function_call()) {
			OUTCOME_TRY(parse_function_call(value));
		}
		else {
			OUTCOME_TRY(parse_expression(value));
		}

		out_node = new assignment_node(m_current_token.get_token_location(), variable, value);
		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_array_access(node*& out_node) {
		get_next_token(); // identifier (guaranteed)
		const std::string identifier = m_current_token.get_value();

		node* array_node = new variable_node(m_current_token.get_token_location(), identifier);
		std::vector<node*> index_nodes;

		get_next_token(); // l_bracket (guaranteed)

		// parse access indices for multiple dimensions
		while (m_current_token.get_token() == token::l_bracket) {
			// parse access index
			node* array_index;

			OUTCOME_TRY(parse_expression(array_index, type(type::base::u64, 0)));
			index_nodes.push_back(array_index);
			OUTCOME_TRY(expect_next_token(token::r_bracket));

			if (peek_next_token() != token::l_bracket) {
				break;
			}

			get_next_token(); // l_bracket(guaranteed)
		}

		out_node = new array_access_node(m_current_token.get_token_location(), array_node, index_nodes);
		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_function_call(node*& out_node) {
		get_next_token(); // identifier (guaranteed)
		const std::string identifier = m_current_token.get_value();
		get_next_token(); // l_parenthesis (guaranteed)
		std::vector<node*> arguments;

		token next_token = peek_next_token();
		if (next_token != token::r_parenthesis) {
			while (true) {
				node* argument;
				OUTCOME_TRY(parse_expression(argument));

				arguments.push_back(argument);

				next_token = peek_next_token(); // comma || r_parenthesis || other

				if (next_token == token::comma) {
					get_next_token(); // comma (guaranteed)
				}
				else if (next_token == token::r_parenthesis) {
					break;
				}
				else {
					return outcome::failure(error::emit<3001>(
						m_current_token.get_token_location(),
						m_current_token.get_token()
					));  // return on failure
				}
			}
		}

		OUTCOME_TRY(expect_next_token(token::r_parenthesis));

		out_node = new function_call_node(m_current_token.get_token_location(), identifier, arguments);
		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_return_statement(node*& out_node) {
		const file_position location = m_current_token.get_token_location();
		get_next_token(); // keyword_return (guaranteed)

		// allow return statements without any expressions
		if(peek_next_token() == token::semicolon) {
			out_node = new return_node(location, nullptr);
		}
		else {
			node* expression;
			OUTCOME_TRY(parse_expression(expression));

			out_node = new return_node(location, expression);
		}
	
		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_declaration(node*& out_node, bool is_global) {
		const file_position location = m_current_token.get_token_location();

		type declaration_type;
		OUTCOME_TRY(parse_type(declaration_type));
		OUTCOME_TRY(expect_next_token(token::identifier));

		const std::string identifier = m_current_token.get_value();

		node* value = nullptr;
		if (peek_next_token() == token::operator_assignment) {
			get_next_token(); // operator_assignment
			OUTCOME_TRY(parse_expression(value, declaration_type));
		}

		if (is_global) {
			out_node = new global_declaration_node(location, declaration_type, identifier, value);
			return outcome::success();
		}

		out_node = new local_declaration_node(location, declaration_type, identifier, value);
		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_expression(node*& out_node, type expression_type) {
		return parse_logical_conjunction(out_node, expression_type);
	}

	outcome::result<void> recursive_descent_parser::parse_logical_conjunction(node*& out_node, type expression_type) {
		node* left;
		OUTCOME_TRY(parse_logical_disjunction(left, expression_type));

		while (peek_next_token() == token::operator_logical_conjunction) {
			get_next_token();
			const token_data op = m_current_token;

			node* right;
			OUTCOME_TRY(parse_logical_disjunction(right, expression_type));

			left = new operator_conjunction_node(op.get_token_location(), left, right);
		}

		out_node = left;
		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_logical_disjunction(node*& out_node, type expression_type) {
		node* left;
		OUTCOME_TRY(parse_comparison(left, expression_type));

		while (peek_next_token() == token::operator_logical_disjunction) {
			get_next_token();
			const token_data op = m_current_token;

			node* right;
			OUTCOME_TRY(parse_comparison(right, expression_type));

			left = new operator_disjunction_node(op.get_token_location(), left, right);
		}

		out_node = left;
		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_comparison(node*& out_node, type expression_type) {
		node* left;
		OUTCOME_TRY(parse_term(left, expression_type));

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
			OUTCOME_TRY(parse_term(right, expression_type));

			switch (op.get_token()) {
			case token::operator_greater_than:
				left = new operator_greater_than_node(m_current_token.get_token_location(), left, right);
				break;
			case token::operator_greater_than_equal_to:
				left = new operator_greater_than_equal_to_node(m_current_token.get_token_location(), left, right);
				break;
			case token::operator_less_than:
				left = new operator_less_than_node(m_current_token.get_token_location(), left, right);
				break;
			case token::operator_less_than_equal_to:
				left = new operator_less_than_equal_to_node(m_current_token.get_token_location(), left, right);
				break;
			case token::operator_equals:
				left = new operator_equals_node(m_current_token.get_token_location(), left, right);
				break;
			case token::operator_not_equals:
				left = new operator_not_equals_node(m_current_token.get_token_location(), left, right);
				break;
			}
		}

		out_node = left;
		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_term(node*& out_node, type expression_type) {
		node* left;
		OUTCOME_TRY(parse_factor(left, expression_type));

		while (
			peek_next_token() == token::operator_addition ||
			peek_next_token() == token::operator_subtraction) {
			get_next_token();
			const token_data op = m_current_token;

			node* right;
			OUTCOME_TRY(parse_factor(right, expression_type));

			switch (op.get_token()) {
			case token::operator_addition:
				left = new operator_addition_node(op.get_token_location(), left, right);
				break;
			case token::operator_subtraction:
				left = new operator_subtraction_node(op.get_token_location(), left, right);
				break;
			}
		}

		out_node = left;
		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_factor(node*& out_node, type expression_type) {
		node* left;
		OUTCOME_TRY(parse_primary(left, expression_type));

		while (
			peek_next_token() == token::operator_multiplication ||
			peek_next_token() == token::operator_division ||
			peek_next_token() == token::operator_modulo ||
			peek_next_token() == token::operator_bitwise_and ||
			peek_next_token() == token::operator_bitwise_or ||
			peek_next_token() == token::operator_bitwise_left_shift ||
			peek_next_token() == token::operator_bitwise_right_shift ||
			peek_next_token() == token::operator_bitwise_xor) {

			get_next_token();
			const token_data op = m_current_token;

			node* right;
			OUTCOME_TRY(parse_primary(right, expression_type));

			switch (op.get_token()) {
			case token::operator_multiplication:
				left = new operator_multiplication_node(op.get_token_location(), left, right);
				break;
			case token::operator_division:
				left = new operator_division_node(op.get_token_location(), left, right);
				break;
			case token::operator_modulo:
				left = new operator_modulo_node(op.get_token_location(), left, right);
				break;
			case token::operator_bitwise_and:
				left = new operator_bitwise_and_node(op.get_token_location(), left, right);
				break;
			case token::operator_bitwise_or:
				left = new operator_bitwise_or_node(op.get_token_location(), left, right);
				break;
			case token::operator_bitwise_left_shift:
				left = new operator_bitwise_left_shift_node(op.get_token_location(), left, right);
				break;
			case token::operator_bitwise_right_shift:
				left = new operator_bitwise_right_shift_node(op.get_token_location(), left, right);
				break;
			case token::operator_bitwise_xor:
				left = new operator_bitwise_xor_node(op.get_token_location(), left, right);
				break;
			}
		}

		out_node = left;
		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_primary(node*& out_node, type expression_type) {
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
		case token::operator_not:
		case token::operator_bitwise_not:
			// check if the next token is an identifier or an opening parenthesis
			token next_next_token;
			next_next_token = peek_nth_token(2);

			if (
				next_next_token == token::identifier || 
				next_next_token == token::l_parenthesis) {
				return parse_pre_operator(out_node);
			}

			return outcome::failure(error::emit<3004>(
				m_current_token.get_token_location()
			)); // return on failure
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

		return outcome::failure(error::emit<3001>(
			m_current_token.get_token_location(),
			m_current_token.get_token()
		)); // return on failure
	}

	outcome::result<void> recursive_descent_parser::parse_number(node*& out_node, type expression_type) {
		get_next_token(); // type
		const std::string str_value = m_current_token.get_value();
		const type ty = expression_type.is_unknown() ? type(m_current_token.get_token(), 0) : expression_type;
		out_node = new numerical_literal_node(m_current_token.get_token_location(), str_value, ty);
		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_char(node*& out_node) {
		get_next_token(); // char_literal (guaranteed)
		out_node = new char_node(m_current_token.get_token_location(), m_current_token.get_value()[0]);
		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_string(node*& out_node) {
		get_next_token(); // string_literal (guaranteed)
		out_node = new string_node(m_current_token.get_token_location(), m_current_token.get_value());
		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_bool(node*& out_node) {
		get_next_token(); // bool_literal_true || bool_literal_false (guaranteed)
		out_node = new bool_node(m_current_token.get_token_location(), m_current_token.get_token() == token::bool_literal_true);
		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_break_keyword(node*& out_node) {
		get_next_token(); // keyword_break (guaranteed)
		out_node = new break_node(m_current_token.get_token_location());
		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_post_operator(node* operand, node*& out_node) {
		get_next_token();

		if (m_current_token.get_token() == token::operator_increment) {
			out_node = new operator_post_increment_node(m_current_token.get_token_location(), operand);
		}
		else {
			out_node = new operator_post_decrement_node(m_current_token.get_token_location(), operand);
		}

		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_pre_operator(node*& out_node) {
		get_next_token();
		const token_data op = m_current_token;
		node* operand;

		OUTCOME_TRY(parse_primary(operand, type::unknown()));

		switch (op.get_token()) {
		case token::operator_increment:
			out_node = new operator_pre_increment_node(op.get_token_location(), operand);
			break;
		case token::operator_decrement:
			out_node = new operator_pre_decrement_node(op.get_token_location(), operand);
			break;
		case token::operator_not:
			out_node = new operator_not_node(op.get_token_location(), operand);
			break;
		case token::operator_bitwise_not:
			out_node = new operator_bitwise_not_node(op.get_token_location(), operand);
			break;
		}

		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_negative_number(node*& out_node, type expression_type) {
		get_next_token(); // operator_subtraction (guaranteed)

		// negate the number by subtracting it from 0
		node* zero_node = create_zero_node(expression_type);
		node* number;

		OUTCOME_TRY(parse_number(number, expression_type));

		out_node = new operator_subtraction_node(m_current_token.get_token_location(), zero_node, number);
		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_new_allocation(node*& out_node) {
		get_next_token(); // keyword_new (guaranteed)
		const file_position location = m_current_token.get_token_location();

		type allocation_type;
		OUTCOME_TRY(parse_type(allocation_type));

		// l_bracket
		OUTCOME_TRY(expect_next_token(token::l_bracket));

		// parse array size
		node* array_size;
		OUTCOME_TRY(parse_expression(array_size, type(type::base::u64, 0)));

		// r_bracket
		OUTCOME_TRY(expect_next_token(token::r_bracket));

		out_node = new array_allocation_node(location, allocation_type, array_size);
		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_primary_identifier(node*& out_node) {
		if (peek_is_function_call()) {
			// parse a function call
			return parse_function_call(out_node);
		}
		else if (peek_is_array_index_access()) {
			OUTCOME_TRY(parse_array_access(out_node));
		}
		else {
			// parse an assignment
			get_next_token();
			const std::string identifier = m_current_token.get_value();
			out_node = new variable_access_node(m_current_token.get_token_location(), identifier);
		}

		const token next_token = peek_next_token();
		// post increment
		if (next_token == token::operator_increment || next_token == token::operator_decrement) {
			return parse_post_operator(out_node, out_node);
		}
		// compound operation
		if(is_token_compound_op(next_token)) {
			return parse_compound_operation(out_node, out_node);
		}

		return outcome::success();
	}

	outcome::result<void> recursive_descent_parser::parse_deep_expression(node*& out_node, type expression_type) {
		get_next_token(); // l_parenthesis (guaranteed)

		// nested expression
		OUTCOME_TRY(parse_expression(out_node, expression_type));

		// r_parenthesis
		OUTCOME_TRY(expect_next_token(token::r_parenthesis));

		return outcome::success();
	}

	bool recursive_descent_parser::peek_is_function_definition() {
		// type
		if (!is_token_type(m_token_list.peek_token().get_token())) {
			m_token_list.synchronize_indices();
			return false; // return on failure
		}

		// pointers?
		token tok = m_token_list.peek_token().get_token();
		while (tok == token::operator_multiplication) {
			tok = m_token_list.peek_token().get_token();
		}

		// identifier
		if (tok != token::identifier) {
			m_token_list.synchronize_indices();
			return false; // return on failure
		}

		const bool result = m_token_list.peek_token().get_token() == token::l_parenthesis;
		m_token_list.synchronize_indices();
		return result;
	}

	bool recursive_descent_parser::peek_is_file_include() {
		// hash
		if (m_token_list.peek_token().get_token() != token::hash) {
			m_token_list.synchronize_indices();
			return false;
		}

		// include
		if (m_token_list.peek_token().get_token() != token::keyword_include) {
			m_token_list.synchronize_indices();
			return false;
		}

		m_token_list.synchronize_indices();
		return true;
	}

	bool recursive_descent_parser::peek_is_function_call() {
		// identifier
		if (m_token_list.peek_token().get_token() != token::identifier) {
			m_token_list.synchronize_indices();
			return false; // return on failure
		}

		const bool result = m_token_list.peek_token().get_token() == token::l_parenthesis;
		m_token_list.synchronize_indices();
		return result;
	}

	bool recursive_descent_parser::peek_is_assignment() {
		// identifier
		if (m_token_list.peek_token().get_token() != token::identifier) {
			m_token_list.synchronize_indices();
			return false; // return on failure
		}

		const bool result = m_token_list.peek_token().get_token() == token::operator_assignment;
		m_token_list.synchronize_indices();
		return result;
	}

	bool recursive_descent_parser::peek_is_array_index_access() {
		// identifier
		if (m_token_list.peek_token().get_token() != token::identifier) {
			m_token_list.synchronize_indices();
			return false; // return on failure
		}

		const bool result = m_token_list.peek_token().get_token() == token::l_bracket;
		m_token_list.synchronize_indices();
		return result;
	}

	bool recursive_descent_parser::peek_is_post_operator() {
		// identifier
		if (m_token_list.peek_token().get_token() != token::identifier) {
			m_token_list.synchronize_indices();
			return false; // return on failure
		}

		const token tok = m_token_list.peek_token().get_token();
		const bool result = tok == token::operator_decrement || tok == token::operator_increment;
		m_token_list.synchronize_indices();
		return result;
	}

	token recursive_descent_parser::peek_next_token() {
		const token_data pair = m_token_list.peek_token();
		m_token_list.synchronize_indices();
		return pair.get_token();
	}

	token recursive_descent_parser::peek_nth_token(u64 offset) {
		for (u64 i = 0; i < offset - 1; ++i) {
			m_token_list.peek_token();
		}

		const token_data pair = m_token_list.peek_token();
		m_token_list.synchronize_indices();
		return pair.get_token();
	}

	node* recursive_descent_parser::create_zero_node(type expression_type) const {
		return new numerical_literal_node(m_current_token.get_token_location(), "0", expression_type);
	}

	outcome::result<void> recursive_descent_parser::parse_type(type& ty) {
		get_next_token();

		if (!is_token_type(m_current_token.get_token())) {
			return outcome::failure(error::emit<3003>(
				m_current_token.get_token_location(),
				m_current_token.get_token()
			)); // return on failure
		}

		ty = type(m_current_token.get_token(), 0);

		// check if the next token is an asterisk
		while (peek_next_token() == token::operator_multiplication) {
			get_next_token(); // operator_multiplication (guaranteed)
			ty.set_pointer_level(ty.get_pointer_level() + 1);
		}

		return outcome::success();
	}
}
