#include "parser.h"
#include "utility/macros.h"

// functions
#include "code_generator/abstract_syntax_tree/functions/function_call_node.h"
#include "code_generator/abstract_syntax_tree/functions/function_node.h"

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
#include "code_generator/abstract_syntax_tree/operators/unary/bitwise/operator_address_of_node.h"
#include "code_generator/abstract_syntax_tree/operators/unary/bitwise/operator_dereference_node.h"
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
	parser::parser(
		const token_list& token_list
	) : m_token_list(token_list),
	m_abstract_syntax_tree(
		std::make_shared<abstract_syntax_tree>()
	) {}

	outcome::result<std::shared_ptr<abstract_syntax_tree>> parser::parse() {
		while (true) {
			if (peek_next_token() == token::end_of_file) {
				return m_abstract_syntax_tree;
			}

			if(peek_is_include_directive()) {
				// parse an include directive
				// just consume the tokens, don't actually do anything with it
				OUTCOME_TRY(parse_include_directive());
				m_include_directive_indices.push_back(m_top_level_abstract_syntax_tree_node_count);
				m_top_level_abstract_syntax_tree_node_count++;
				continue;
			}

			node_ptr node;
			if (peek_is_function_definition()) {
				// parse a top-level function definition
				OUTCOME_TRY(node, parse_function_definition());
			}
			else {
				// parse a global statement
				OUTCOME_TRY(node, parse_global_statement());
			}

			m_abstract_syntax_tree->add_node(node);
			m_top_level_abstract_syntax_tree_node_count++;
		}
	}

	std::shared_ptr<abstract_syntax_tree> parser::get_abstract_syntax_tree() const {
		return m_abstract_syntax_tree;
	}

	const std::vector<u64>& parser::get_include_directive_indices() const {
		return m_include_directive_indices;
	}

	outcome::result<node_ptr> parser::parse_function_definition() {
		OUTCOME_TRY(const type return_type, parse_type());

		const file_position position = m_token_list.get_current_token().get_position();
		m_token_list.get_token(); // identifier (guaranteed)
		const std::string identifier = m_token_list.get_current_token().get_value();
		m_token_list.get_token(); // l_parenthesis (guaranteed)

		std::vector<std::pair<std::string, type>> arguments;

		// parse arguments
		token next_token = peek_next_token();
		if (next_token != token::r_parenthesis) {
			while (true) {
				OUTCOME_TRY(type argument_type, parse_type());
				OUTCOME_TRY(m_token_list.expect_token(token::identifier));

				std::string argument_name = m_token_list.get_current_token().get_value();
				arguments.emplace_back(argument_name, argument_type);

				// m_token_list.get_token(); // comma || type || other
				next_token = peek_next_token();
				if (next_token == token::comma) {
					m_token_list.get_token(); // comma (guaranteed)
				}
				else if (next_token != token::r_parenthesis) {
					return outcome::failure(error::emit<3000>(
						m_token_list.get_current_token().get_position(),
						token::r_parenthesis,
						m_token_list.get_current_token().get_token()
					)); // return on failure
				}
				else {
					m_token_list.get_token(); // r_parenthesis(guaranteed)
					break;
				}
			}
		}
		else {
			m_token_list.get_token(); // r_parenthesis (guaranteed)
		}

		OUTCOME_TRY(const std::vector<node_ptr>&statements, parse_local_statements());

		return new function_node(
			position,
			return_type,
			false,
			identifier,
			arguments,
			statements
		);
	}

	outcome::result<node_ptr> parser::parse_global_statement() {
		const token token = peek_next_token(); // identifier | type | keyword
		node_ptr global_statement_node;

		if (is_token_type(token)) {
			// statements beginning with a type keyword have to be variable declarations
			OUTCOME_TRY(global_statement_node, parse_declaration(true));
		}
		else {
			if (peek_is_assignment()) {
				OUTCOME_TRY(global_statement_node, parse_assignment());
			}
			else {
				m_token_list.get_token(); // read the erroneous token

				return outcome::failure(error::emit<3001>(
					m_token_list.get_current_token().get_position(),
					token
				));
			}
		}

		OUTCOME_TRY(m_token_list.expect_token(token::semicolon));
		return global_statement_node;
	}

	outcome::result<void> parser::parse_include_directive() {
		m_token_list.get_token(); // hash (guaranteed)
		m_token_list.get_token(); // keyword_include (guaranteed)
		OUTCOME_TRY(m_token_list.expect_token(token::string_literal));
		const std::string include_str = m_token_list.get_current_token().get_value();
		outcome::success();
	}

	outcome::result<std::vector<node_ptr>> parser::parse_local_statements() {
		OUTCOME_TRY(m_token_list.expect_token(token::l_brace));

		std::vector<node_ptr> local_statement_nodes;
		bool met_block_break = false;
		token next_token = peek_next_token();

		// get all statements in the current scope
		while (next_token != token::r_brace) {
			OUTCOME_TRY(node_ptr statement, parse_local_statement());

			// check if we've met a block break token
			if (met_block_break == false) {
				met_block_break = is_token_block_break(next_token);
				// if we haven't, we can add the statement to the vector
				// this prevents us from adding unreachable nodes to the AST
				local_statement_nodes.push_back(statement);
			}

			next_token = peek_next_token();
		}

		m_token_list.get_token(); // r_brace (guaranteed)
		return local_statement_nodes;
	}

	outcome::result<node_ptr> parser::parse_local_statement() {
		const token next_token = peek_next_token(); // identifier || type || keyword
		node_ptr local_statement_node;

		if (is_token_type(next_token)) {
			// statements beginning with a type keyword have to be variable declarations
			OUTCOME_TRY(local_statement_node, parse_declaration(false));
		}
		else {
			switch (next_token) {
			case token::asterisk:
			case token::identifier: {
				OUTCOME_TRY(local_statement_node, parse_local_statement_identifier());
				break;
			}
			case token::l_parenthesis: {
				// parse a deep expression (parenthesized expression)
				OUTCOME_TRY(local_statement_node, parse_deep_expression(type::unknown()));

				// check for post unary operators after deep expression
				if (peek_next_token() == token::operator_increment || peek_next_token() == token::operator_decrement) {
					OUTCOME_TRY(local_statement_node, parse_post_operator(local_statement_node));
				}
				break;
			}
			case token::operator_increment:
			case token::operator_decrement:
			case token::operator_not:
			case token::operator_bitwise_not:
			case token::ampersand: {
				// check if the next token is an identifier or an opening parenthesis
				const token next_next_token = peek_nth_token(2);

				if (next_next_token == token::identifier || next_next_token == token::l_parenthesis) {
					OUTCOME_TRY(local_statement_node, parse_pre_operator());
				}
				else {
					return outcome::failure(error::emit<3004>(
						m_token_list.get_current_token().get_position()
					)); // return on failure
				}

				break;
			}
			case token::keyword_return: {
				OUTCOME_TRY(local_statement_node, parse_return_statement());
				break;
			}
			case token::keyword_break: {
				OUTCOME_TRY(local_statement_node, parse_break_keyword());
				break;
			}
			case token::keyword_if:
				// return right away since we don't want to check for a semicolon at the end of the statement
				return parse_if_else_statement();
			case token::keyword_while:
				// return right away since we don't want to check for a semicolon at the end of the statement
				return parse_while_loop();
			case token::keyword_for:
				// return right away since we don't want to check for a semicolon at the end of the statement
				return parse_for_loop();
			default:
				return outcome::failure(error::emit<3001>(
					m_token_list.get_current_token().get_position(),
					next_token
				)); // return on failure
			}
		}

		OUTCOME_TRY(m_token_list.expect_token(token::semicolon));
		return local_statement_node;
	}

	outcome::result<node_ptr> parser::parse_local_statement_identifier() {
		node_ptr local_statement_identifier_node;

		if (peek_is_function_call()) {
			// function call statement
			OUTCOME_TRY(local_statement_identifier_node, parse_function_call());
		}
		else if (peek_is_array_index_access()) {
			// array assignment
			OUTCOME_TRY(local_statement_identifier_node, parse_array_assignment());
		}
		else if (peek_is_assignment()) {
			// assignment statement
			OUTCOME_TRY(local_statement_identifier_node, parse_assignment());
		}
		else {
			// create a simple access node
			OUTCOME_TRY(local_statement_identifier_node, parse_variable_access());
		}

		// check for post unary operators after identifier, deep expression, or array index access
		const token next_token = peek_next_token();
		if (next_token == token::operator_increment || next_token == token::operator_decrement) {
			return parse_post_operator(local_statement_identifier_node);
		}

		// compound operation
		if (is_token_compound_op(next_token)) {
			return parse_compound_operation(local_statement_identifier_node);
		}

		return local_statement_identifier_node;
	}

	outcome::result<node_ptr> parser::parse_if_else_statement() {
		std::vector<node_ptr> conditions;
		std::vector<std::vector<node_ptr>> branches;
		const file_position position = m_token_list.get_current_token().get_position();
		bool has_else = false;

		while (true) {
			if (!has_else && peek_next_token() == token::keyword_else) {
				m_token_list.get_token(); // keyword_else (guaranteed)
				if (peek_next_token() != token::keyword_if) {
					has_else = true;
				}
			}

			if (peek_next_token() == token::keyword_if || has_else) {
				if (!has_else) {
					m_token_list.get_token(); // keyword_if (guaranteed)
				}

				node_ptr condition;
				if (!has_else) {
					OUTCOME_TRY(m_token_list.expect_token(token::l_parenthesis));
					OUTCOME_TRY(condition, parse_expression());
					OUTCOME_TRY(m_token_list.expect_token(token::r_parenthesis));
				}

				conditions.push_back(condition);
			}
			else {
				break;
			}

			OUTCOME_TRY(const std::vector<node_ptr>&branch_statements, parse_local_statements());
			branches.push_back(branch_statements);

			if (has_else) {
				break;
			}
		}

		return new if_else_node(position, conditions, branches);
	}

	outcome::result<node_ptr> parser::parse_while_loop() {
		m_token_list.get_token(); // keyword_while (guaranteed)
		const file_position position = m_token_list.get_current_token().get_position();

		OUTCOME_TRY(m_token_list.expect_token(token::l_parenthesis));

		OUTCOME_TRY(const node_ptr loop_condition_node, parse_expression());
		OUTCOME_TRY(m_token_list.expect_token(token::r_parenthesis));
		OUTCOME_TRY(m_token_list.expect_token(token::l_brace));

		std::vector<node_ptr> loop_statements;
		while (peek_next_token() != token::r_brace) {
			OUTCOME_TRY(node_ptr statement, parse_local_statement());

			loop_statements.push_back(statement);
		}

		m_token_list.get_token(); // r_brace (guaranteed)
		return new while_node(position, loop_condition_node, loop_statements);
	}

	outcome::result<node_ptr> parser::parse_loop_increment() {
		node_ptr loop_increment_node;

		switch (const token next_token = peek_next_token()) {
		case token::identifier: {
			OUTCOME_TRY(loop_increment_node, parse_local_statement_identifier());
			break;
		}
		case token::operator_increment:
		case token::operator_decrement:
		case token::operator_not:
		case token::operator_bitwise_not:
		case token::ampersand:
			// check if the next token is an identifier or an opening parenthesis
			token next_next_token;
			next_next_token = peek_nth_token(2);

			if (
				next_next_token == token::identifier ||
				next_next_token == token::l_parenthesis) {
				OUTCOME_TRY(loop_increment_node, parse_pre_operator());
			}
			else {
				return outcome::failure(error::emit<3004>(
					m_token_list.get_current_token().get_position()
				)); // return on failure
			}
			break;
		case token::r_parenthesis:
			break;
		default:
			return outcome::failure(error::emit<3001>(
				m_token_list.get_current_token().get_position(),
				next_token
			)); // return on failure
		}

		return loop_increment_node;
	}

	outcome::result<node_ptr> parser::parse_for_loop() {
		m_token_list.get_token(); // keyword_for (guaranteed)
		const file_position position = m_token_list.get_current_token().get_position();

		OUTCOME_TRY(m_token_list.expect_token(token::l_parenthesis));

		// parse the initialization section
		node_ptr loop_initialization_node;
		const token next_token = peek_next_token();

		if (is_token_type(next_token)) {
			// statements beginning with a type keyword have to be variable declarations
			OUTCOME_TRY(loop_initialization_node, parse_declaration(false));
		}
		else if (next_token == token::identifier) {
			OUTCOME_TRY(loop_initialization_node, parse_local_statement_identifier());
		}
		else {
			return outcome::failure(error::emit<3001>(
				m_token_list.get_current_token().get_position(),
				next_token
			)); // return on failure
		}

		OUTCOME_TRY(m_token_list.expect_token(token::semicolon));

		// parse the loop condition section
		OUTCOME_TRY(node_ptr loop_condition_node, parse_expression());
		OUTCOME_TRY(m_token_list.expect_token(token::semicolon));

		// parse the increment section
		std::vector<node_ptr> post_iteration_nodes;

		while (true) {
			if (peek_next_token() == token::r_parenthesis) {
				break;
			}

			OUTCOME_TRY(node_ptr post_iteration_node, parse_loop_increment());
			post_iteration_nodes.push_back(post_iteration_node);

			if (peek_next_token() != token::comma) {
				break;
			}

			m_token_list.get_token(); // comma (guaranteed)
		}

		OUTCOME_TRY(m_token_list.expect_token(token::r_parenthesis));
		OUTCOME_TRY(m_token_list.expect_token(token::l_brace));

		// parse body statements
		std::vector<node_ptr> loop_statements;
		while (peek_next_token() != token::r_brace) {
			OUTCOME_TRY(node_ptr statement, parse_local_statement());
			loop_statements.push_back(statement);
		}

		m_token_list.get_token(); // r_brace (guaranteed)
		return new for_node(
			position,
			loop_initialization_node,
			loop_condition_node,
			post_iteration_nodes,
			loop_statements
		);
	}

	outcome::result<node_ptr> parser::parse_compound_operation(node_ptr left_operand) {
		// operator_addition_assignment ||
		// operator_subtraction_assignment || 
		// operator_multiplication_assignment || 
		// operator_modulo_assignment || 
		// operator_division_assignment (guaranteed)
		m_token_list.get_token();
		const token op = m_token_list.get_current_token().get_token();

		// rhs of the expression
		OUTCOME_TRY(const node_ptr expression, parse_expression());

		switch (op) {
		case token::operator_addition_assignment:
			return new operator_addition_assignment_node(
				m_token_list.get_current_token().get_position(),
				left_operand,
				expression
			);
		case token::operator_subtraction_assignment:
			return new operator_subtraction_assignment_node(
				m_token_list.get_current_token().get_position(),
				left_operand,
				expression
			);
		case token::operator_multiplication_assignment:
			return new operator_multiplication_assignment_node(
				m_token_list.get_current_token().get_position(),
				left_operand,
				expression
			);
		case token::operator_division_assignment:
			return new operator_division_assignment_node(
				m_token_list.get_current_token().get_position(),
				left_operand,
				expression
			);
		case token::operator_modulo_assignment:
			return new operator_modulo_assignment_node(
				m_token_list.get_current_token().get_position(),
				left_operand,
				expression
			);
		default:
			ASSERT(false, "invalid compound operator received");
		}

		return nullptr;
	}

	outcome::result<node_ptr> parser::parse_array_assignment() {
		m_token_list.get_token(); // identifier (guaranteed)
		const std::string identifier = m_token_list.get_current_token().get_value();
		const file_position position = m_token_list.get_current_token().get_position();

		std::vector<node_ptr> index_nodes;
		while (peek_next_token() == token::l_bracket) {
			m_token_list.get_token(); // l_bracket (guaranteed)
			OUTCOME_TRY(node_ptr index_node, parse_expression(type(type::base::u64, 0)));

			index_nodes.push_back(index_node);

			// make sure the next token is a right square bracket
			OUTCOME_TRY(m_token_list.expect_token(token::r_bracket));
		}

		const token next_token = peek_next_token();
		if (next_token == token::operator_assignment) {
			m_token_list.get_token();

			// parse access-assignment
			node_ptr value;
			if (peek_is_function_call()) {
				OUTCOME_TRY(value, parse_function_call());
			}
			else {
				OUTCOME_TRY(value, parse_expression());
			}

			const node_ptr array_node = new variable_node(m_token_list.get_current_token().get_position(), identifier);
			return new array_assignment_node(position, array_node, index_nodes, value);
		}

		if (next_token == token::operator_increment || next_token == token::operator_decrement) {
			const node_ptr array_node = new variable_node(m_token_list.get_current_token().get_position(), identifier);
			node_ptr array_access = new array_access_node(position, array_node, index_nodes);

			OUTCOME_TRY(array_access, parse_post_operator(array_access));
		}

		return nullptr;
	}

	outcome::result<node_ptr> parser::parse_variable_access() {
		const token next = peek_next_token();

		if(next == token::asterisk) {
			m_token_list.get_token(); // asterisk (guaranteed)
			OUTCOME_TRY(const node_ptr node, parse_variable_access());

			return new operator_dereference_node(
				m_token_list.get_current_token().get_position(),
				node
			);
		}

		m_token_list.get_token();
		return new variable_access_node(
			m_token_list.get_current_token().get_position(),
			m_token_list.get_current_token().get_value()
		);
	}

	outcome::result<node_ptr> parser::parse_variable() {
		const token next = peek_next_token();

		if (next == token::asterisk) {
			m_token_list.get_token(); // asterisk (guaranteed)
			OUTCOME_TRY(const node_ptr node, parse_variable_access());

			return new operator_dereference_node(
				m_token_list.get_current_token().get_position(),
				node
			);
		}

		m_token_list.get_token();
		return new variable_node(
			m_token_list.get_current_token().get_position(),
			m_token_list.get_current_token().get_value()
		);
	}

	outcome::result<node_ptr> parser::parse_assignment() {
		OUTCOME_TRY(const node_ptr variable, parse_variable());
		OUTCOME_TRY(m_token_list.expect_token(token::operator_assignment));

		// parse access-assignment
		node_ptr value;
		if (peek_is_function_call()) {
			OUTCOME_TRY(value, parse_function_call());
		}
		else {
			OUTCOME_TRY(value, parse_expression());
		}

		return new assignment_node(m_token_list.get_current_token().get_position(), variable, value);
	}

	outcome::result<node_ptr> parser::parse_array_access() {
		m_token_list.get_token(); // identifier (guaranteed)
		const std::string identifier = m_token_list.get_current_token().get_value();

		const node_ptr array_node = new variable_node(m_token_list.get_current_token().get_position(), identifier);
		std::vector<node_ptr> index_nodes;

		m_token_list.get_token(); // l_bracket (guaranteed)

		// parse access indices for multiple dimensions
		while (m_token_list.get_current_token().get_token() == token::l_bracket) {
			// parse access index
			OUTCOME_TRY(node_ptr array_index, parse_expression(type(type::base::u64, 0)));
			index_nodes.push_back(array_index);
			OUTCOME_TRY(m_token_list.expect_token(token::r_bracket));

			if (peek_next_token() != token::l_bracket) {
				break;
			}

			m_token_list.get_token(); // l_bracket(guaranteed)
		}

		return new array_access_node(m_token_list.get_current_token().get_position(), array_node, index_nodes);
	}

	outcome::result<node_ptr> parser::parse_function_call() {
		m_token_list.get_token(); // identifier (guaranteed)
		const std::string identifier = m_token_list.get_current_token().get_value();
		m_token_list.get_token(); // l_parenthesis (guaranteed)
		std::vector<node_ptr> arguments;

		token next_token = peek_next_token();
		if (next_token != token::r_parenthesis) {
			while (true) {
				OUTCOME_TRY(node_ptr argument, parse_expression());
				arguments.push_back(argument);

				next_token = peek_next_token(); // comma || r_parenthesis || other

				if (next_token == token::comma) {
					m_token_list.get_token(); // comma (guaranteed)
				}
				else if (next_token == token::r_parenthesis) {
					break;
				}
				else {
					return outcome::failure(error::emit<3001>(
						m_token_list.get_current_token().get_position(),
						m_token_list.get_current_token().get_token()
					));  // return on failure
				}
			}
		}

		OUTCOME_TRY(m_token_list.expect_token(token::r_parenthesis));
		return new function_call_node(m_token_list.get_current_token().get_position(), identifier, arguments);
	}

	outcome::result<node_ptr> parser::parse_return_statement() {
		const file_position position = m_token_list.get_current_token().get_position();
		m_token_list.get_token(); // keyword_return (guaranteed)

		// allow return statements without any expressions
		if (peek_next_token() == token::semicolon) {
			return new return_node(position, nullptr);
		}

		OUTCOME_TRY(const node_ptr expression, parse_expression());
		return new return_node(position, expression);
	}

	outcome::result<node_ptr> parser::parse_declaration(bool is_global) {
		const file_position position = m_token_list.get_current_token().get_position();

		OUTCOME_TRY(const type declaration_type, parse_type());
		OUTCOME_TRY(m_token_list.expect_token(token::identifier));

		const std::string identifier = m_token_list.get_current_token().get_value();

		node_ptr value = nullptr;
		if (peek_next_token() == token::operator_assignment) {
			m_token_list.get_token(); // operator_assignment
			OUTCOME_TRY(value, parse_expression(declaration_type));
		}

		if (is_global) {
			return new global_declaration_node(position, declaration_type, identifier, value);
		}

		return new local_declaration_node(position, declaration_type, identifier, value);
	}

	outcome::result<node_ptr> parser::parse_expression(type expression_type) {
		return parse_logical_conjunction(expression_type);
	}

	outcome::result<node_ptr> parser::parse_logical_conjunction(type expression_type) {
		OUTCOME_TRY(node_ptr left, parse_logical_disjunction(expression_type));

		while (peek_next_token() == token::operator_logical_conjunction) {
			m_token_list.get_token();

			const token_data op = m_token_list.get_current_token();

			OUTCOME_TRY(node_ptr right, parse_logical_disjunction(expression_type));
			left = new operator_conjunction_node(op.get_position(), left, right);
		}

		return left;
	}

	outcome::result<node_ptr> parser::parse_logical_disjunction(type expression_type) {
		OUTCOME_TRY(node_ptr left, parse_comparison(expression_type));

		while (peek_next_token() == token::operator_logical_disjunction) {
			m_token_list.get_token();

			const token_data op = m_token_list.get_current_token();

			OUTCOME_TRY(node_ptr right, parse_comparison(expression_type));
			left = new operator_disjunction_node(op.get_position(), left, right);
		}

		return left;
	}

	outcome::result<node_ptr> parser::parse_comparison(type expression_type) {
		OUTCOME_TRY(node_ptr left, parse_term(expression_type));

		while (
			peek_next_token() == token::operator_greater_than ||
			peek_next_token() == token::operator_greater_than_equal_to ||
			peek_next_token() == token::operator_less_than ||
			peek_next_token() == token::operator_less_than_equal_to ||
			peek_next_token() == token::operator_equals ||
			peek_next_token() == token::operator_not_equals) {
			m_token_list.get_token();
			const token_data op = m_token_list.get_current_token();

			OUTCOME_TRY(node_ptr right, parse_term(expression_type));

			switch (op.get_token()) {
			case token::operator_greater_than:
				left = new operator_greater_than_node(m_token_list.get_current_token().get_position(), left, right);
				break;
			case token::operator_greater_than_equal_to:
				left = new operator_greater_than_equal_to_node(m_token_list.get_current_token().get_position(), left, right);
				break;
			case token::operator_less_than:
				left = new operator_less_than_node(m_token_list.get_current_token().get_position(), left, right);
				break;
			case token::operator_less_than_equal_to:
				left = new operator_less_than_equal_to_node(m_token_list.get_current_token().get_position(), left, right);
				break;
			case token::operator_equals:
				left = new operator_equals_node(m_token_list.get_current_token().get_position(), left, right);
				break;
			case token::operator_not_equals:
				left = new operator_not_equals_node(m_token_list.get_current_token().get_position(), left, right);
				break;
			}
		}

		return left;
	}

	outcome::result<node_ptr> parser::parse_term(type expression_type) {
		OUTCOME_TRY(node_ptr left, parse_factor(expression_type));

		while (
			peek_next_token() == token::operator_addition ||
			peek_next_token() == token::operator_subtraction) {
			m_token_list.get_token();
			const token_data op = m_token_list.get_current_token();

			OUTCOME_TRY(node_ptr right, parse_factor(expression_type));

			switch (op.get_token()) {
			case token::operator_addition:
				left = new operator_addition_node(op.get_position(), left, right);
				break;
			case token::operator_subtraction:
				left = new operator_subtraction_node(op.get_position(), left, right);
				break;
			}
		}

		return left;
	}

	outcome::result<node_ptr> parser::parse_factor(type expression_type) {
		OUTCOME_TRY(node_ptr left, parse_primary(expression_type));

		while (
			peek_next_token() == token::asterisk ||
			peek_next_token() == token::operator_division ||
			peek_next_token() == token::operator_modulo ||
			peek_next_token() == token::ampersand ||
			peek_next_token() == token::operator_bitwise_or ||
			peek_next_token() == token::operator_bitwise_left_shift ||
			peek_next_token() == token::operator_bitwise_right_shift ||
			peek_next_token() == token::operator_bitwise_xor) {

			m_token_list.get_token();
			const token_data op = m_token_list.get_current_token();
			OUTCOME_TRY(node_ptr right, parse_primary(expression_type));

			switch (op.get_token()) {
			case token::asterisk:
				left = new operator_multiplication_node(op.get_position(), left, right);
				break;
			case token::operator_division:
				left = new operator_division_node(op.get_position(), left, right);
				break;
			case token::operator_modulo:
				left = new operator_modulo_node(op.get_position(), left, right);
				break;
			case token::ampersand:
				left = new operator_bitwise_and_node(op.get_position(), left, right);
				break;
			case token::operator_bitwise_or:
				left = new operator_bitwise_or_node(op.get_position(), left, right);
				break;
			case token::operator_bitwise_left_shift:
				left = new operator_bitwise_left_shift_node(op.get_position(), left, right);
				break;
			case token::operator_bitwise_right_shift:
				left = new operator_bitwise_right_shift_node(op.get_position(), left, right);
				break;
			case token::operator_bitwise_xor:
				left = new operator_bitwise_xor_node(op.get_position(), left, right);
				break;
			}
		}

		return left;
	}

	outcome::result<node_ptr> parser::parse_primary(type expression_type) {
		const token next_token = peek_next_token();

		if (is_token_numerical(next_token)) {
			// parse a number
			return parse_number(expression_type);
		}

		switch (next_token) {
		case token::operator_subtraction:
			return parse_negative_number(expression_type);
		case token::operator_increment:
		case token::operator_decrement:
		case token::operator_not:
		case token::operator_bitwise_not:
		case token::ampersand:
			// check if the next token is an identifier or an opening parenthesis
			token next_next_token;
			next_next_token = peek_nth_token(2);

			if (
				next_next_token == token::identifier ||
				next_next_token == token::l_parenthesis) {
				return parse_pre_operator();
			}

			return outcome::failure(error::emit<3004>(
				m_token_list.get_current_token().get_position()
			)); // return on failure
		case token::identifier:
			// parse a function call or an assignment
			return parse_primary_identifier();
		case token::l_parenthesis:
			// parse a deep expression
			return parse_deep_expression(expression_type);
		case token::keyword_new:
			// parse an allocation
			return parse_new_allocation();
		case token::char_literal:
			// parse a char literal
			return parse_char();
		case token::string_literal:
			// parse a string literal
			return parse_string();
		case token::bool_literal_true:
		case token::bool_literal_false:
			// parse a boolean
			return parse_bool();
		}

		return outcome::failure(error::emit<3001>(
			m_token_list.get_current_token().get_position(),
			m_token_list.get_current_token().get_token()
		)); // return on failure
	}

	outcome::result<node_ptr> parser::parse_number(type expression_type) {
		m_token_list.get_token(); // type
		const std::string str_value = m_token_list.get_current_token().get_value();
		const type ty = expression_type.is_unknown() ? type(m_token_list.get_current_token().get_token(), 0) : expression_type;
		return new numerical_literal_node(m_token_list.get_current_token().get_position(), str_value, ty);
	}

	outcome::result<node_ptr> parser::parse_char() {
		m_token_list.get_token(); // char_literal (guaranteed)
		return new char_node(m_token_list.get_current_token().get_position(), m_token_list.get_current_token().get_value()[0]);
	}

	outcome::result<node_ptr> parser::parse_string() {
		m_token_list.get_token(); // string_literal (guaranteed)
		return new string_node(m_token_list.get_current_token().get_position(), m_token_list.get_current_token().get_value());
	}

	outcome::result<node_ptr> parser::parse_bool() {
		m_token_list.get_token(); // bool_literal_true || bool_literal_false (guaranteed)
		return new bool_node(m_token_list.get_current_token().get_position(), m_token_list.get_current_token().get_token() == token::bool_literal_true);
	}

	outcome::result<node_ptr> parser::parse_break_keyword() {
		m_token_list.get_token(); // keyword_break (guaranteed)
		return new break_node(m_token_list.get_current_token().get_position());
	}

	outcome::result<node_ptr> parser::parse_post_operator(node_ptr operand) {
		m_token_list.get_token();

		if (m_token_list.get_current_token().get_token() == token::operator_increment) {
			return new operator_post_increment_node(m_token_list.get_current_token().get_position(), operand);
		}

		return new operator_post_decrement_node(m_token_list.get_current_token().get_position(), operand);
	}

	outcome::result<node_ptr> parser::parse_pre_operator() {
		m_token_list.get_token();
		const token_data op = m_token_list.get_current_token();
		OUTCOME_TRY(const node_ptr operand, parse_primary(type::unknown()));

		switch (op.get_token()) {
		case token::operator_increment:
			return new operator_pre_increment_node(op.get_position(), operand);
		case token::operator_decrement:
			return new operator_pre_decrement_node(op.get_position(), operand);
		case token::operator_not:
			return new operator_not_node(op.get_position(), operand);
		case token::operator_bitwise_not:
			return new operator_bitwise_not_node(op.get_position(), operand);
		case token::ampersand:
			return new operator_address_of_node(op.get_position(), operand);
		}

		ASSERT(false, "unexpected control path reached");
		return nullptr;
	}

	outcome::result<node_ptr> parser::parse_negative_number(type expression_type) {
		m_token_list.get_token(); // operator_subtraction (guaranteed)

		// negate the number by subtracting it from 0
		const node_ptr zero_node = create_zero_node(expression_type);
		OUTCOME_TRY(const node_ptr number, parse_number(expression_type));

		return new operator_subtraction_node(m_token_list.get_current_token().get_position(), zero_node, number);
	}

	outcome::result<node_ptr> parser::parse_new_allocation() {
		m_token_list.get_token(); // keyword_new (guaranteed)
		const file_position position = m_token_list.get_current_token().get_position();
		OUTCOME_TRY(const type allocation_type, parse_type());

		// l_bracket
		OUTCOME_TRY(m_token_list.expect_token(token::l_bracket));

		// parse array size
		OUTCOME_TRY(const node_ptr array_size, parse_expression(type(type::base::u64, 0)));

		// r_bracket
		OUTCOME_TRY(m_token_list.expect_token(token::r_bracket));
		return new array_allocation_node(position, allocation_type, array_size);
	}

	outcome::result<node_ptr> parser::parse_primary_identifier() {
		node_ptr primary_identifier_node;

		if (peek_is_function_call()) {
			// parse a function call
			return parse_function_call();
		}

		if (peek_is_array_index_access()) {
			OUTCOME_TRY(primary_identifier_node, parse_array_access());
		}
		else {
			OUTCOME_TRY(primary_identifier_node, parse_variable_access());
			// parse an assignment
			// m_token_list.get_token();
			// const std::string identifier = m_token_list.get_current_token().get_value();
			// primary_identifier_node = new variable_access_node(m_token_list.get_current_token().get_position(), identifier);
		}

		const token next_token = peek_next_token();
		// post increment
		if (next_token == token::operator_increment || next_token == token::operator_decrement) {
			return parse_post_operator(primary_identifier_node);
		}
		// compound operation
		if (is_token_compound_op(next_token)) {
			return parse_compound_operation(primary_identifier_node);
		}

		return primary_identifier_node;
	}

	outcome::result<node_ptr> parser::parse_deep_expression(type expression_type) {
		m_token_list.get_token(); // l_parenthesis (guaranteed)

		// nested expression
		OUTCOME_TRY(const node_ptr deep_expression_node, parse_expression(expression_type));

		// r_parenthesis
		OUTCOME_TRY(m_token_list.expect_token(token::r_parenthesis));

		return deep_expression_node;
	}

	bool parser::peek_is_function_definition() {
		// type
		if (!is_token_type(m_token_list.peek_token().get_token())) {
			m_token_list.synchronize_indices();
			return false; // return on failure
		}

		// pointers?
		token tok = m_token_list.peek_token().get_token();
		while (tok == token::asterisk) {
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

	bool parser::peek_is_function_call() {
		// identifier
		if (m_token_list.peek_token().get_token() != token::identifier) {
			m_token_list.synchronize_indices();
			return false; // return on failure
		}

		const bool result = m_token_list.peek_token().get_token() == token::l_parenthesis;
		m_token_list.synchronize_indices();
		return result;
	}

	bool parser::peek_is_assignment() {
		token tok = m_token_list.peek_token().get_token();

		// parse all available asterisks (***value = ...)
		while(tok == token::asterisk) {
			tok = m_token_list.peek_token().get_token();
		}

		// identifier
		if(tok != token::identifier) {
			m_token_list.synchronize_indices();
			return false;
		}

		const bool result = m_token_list.peek_token().get_token() == token::operator_assignment;
		m_token_list.synchronize_indices();
		return result;
	}

	bool parser::peek_is_array_index_access() {
		// identifier
		if (m_token_list.peek_token().get_token() != token::identifier) {
			m_token_list.synchronize_indices();
			return false; // return on failure
		}

		const bool result = m_token_list.peek_token().get_token() == token::l_bracket;
		m_token_list.synchronize_indices();
		return result;
	}

	bool parser::peek_is_post_operator() {
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

	bool parser::peek_is_include_directive() {
		if(m_token_list.peek_token().get_token() != token::hash) {
			m_token_list.synchronize_indices();
			return false; // return on failure
		}

		if(m_token_list.peek_token().get_token() != token::keyword_include) {
			m_token_list.synchronize_indices();
			return false; // return on failure
		}

		m_token_list.synchronize_indices();
		return true;
	}

	token parser::peek_next_token() {
		const token_data pair = m_token_list.peek_token();
		m_token_list.synchronize_indices();
		return pair.get_token();
	}

	token parser::peek_nth_token(u64 offset) {
		for (u64 i = 0; i < offset - 1; ++i) {
			m_token_list.peek_token();
		}

		const token_data pair = m_token_list.peek_token();
		m_token_list.synchronize_indices();
		return pair.get_token();
	}

	node_ptr parser::create_zero_node(type expression_type) const {
		return new numerical_literal_node(m_token_list.get_current_token().get_position(), "0", expression_type);
	}

	outcome::result<type> parser::parse_type() {
		m_token_list.get_token();

		if (!is_token_type(m_token_list.get_current_token().get_token())) {
			return outcome::failure(error::emit<3003>(
				m_token_list.get_current_token().get_position(),
				m_token_list.get_current_token().get_token()
			)); // return on failure
		}

		type ty = type(m_token_list.get_current_token().get_token(), 0);

		// check if the next token is an asterisk
		while (peek_next_token() == token::asterisk) {
			m_token_list.get_token(); // operator_multiplication (guaranteed)
			ty.set_pointer_level(ty.get_pointer_level() + 1);
		}

		return ty;
	}

}
