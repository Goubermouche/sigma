#include "parser.h"

#include "../codegen/abstract_syntax_tree/keywords/assignment_node.h"
#include "../codegen/abstract_syntax_tree/keywords/declaration_node.h"
#include "../codegen/abstract_syntax_tree/keywords/function_call_node.h"
#include "../codegen/abstract_syntax_tree/keywords/variable_node.h"
#include "../codegen/abstract_syntax_tree/keywords/types/keyword_i32_node.h"
#include "../codegen/abstract_syntax_tree/operators/operator_addition_node.h"
#include "../codegen/abstract_syntax_tree/operators/operator_division_node.h"
#include "../codegen/abstract_syntax_tree/operators/operator_multiplication_node.h"
#include "../codegen/abstract_syntax_tree/operators/operator_subtraction_node.h"

namespace channel {
	parser::parser(const std::string& source_file)
		: m_lexer(source_file) {
		token token;

		while ((token = m_lexer.get_token()) != token::end_of_file) {
			std::cout << token_to_string(token);

			if (token == token::identifier) {
				std::cout << " (" << m_lexer.get_identifier() << ')';
			}
			else if (token == token::number_signed ||
				token == token::number_unsigned ||
				token == token::number_f32 ||
				token == token::number_f64) {
				std::cout << " (" << m_lexer.get_value() << ')';
			}

			std::cout << '\n';
		}
	}

	std::vector<node*> parser::parse() {
		std::vector<node*> program;

		consume_next_token();
		while (m_current_token != token::end_of_file) {
			node* statement = parse_statement();
			if (statement != nullptr) {
				program.push_back(statement);
			}
		}

		return program;
	}

	node* parser::parse_statement() {
		switch (m_current_token) {
		case token::keyword_type_i32:
			return parse_declaration_or_assignment();
		default:
			std::cout << "unhandled token (" << token_to_string(m_current_token) << ") \n";
			break;
		}
	}

	void parser::consume_next_token() {
		m_current_token = m_lexer.get_token();
	}

	void parser::expect_next_token(token token)	{
		if(m_current_token == token) {
			consume_next_token();
		}
		else {
			ASSERT(false, "unexpected token");
		}
	}

	node* parser::parse_declaration_or_assignment() {
		bool is_declaration = m_current_token == token::keyword_type_i32;
		consume_next_token();

		if (m_current_token != token::identifier) {
			ASSERT(false, std::string("[parser]: expected identifier (got " + token_to_string(m_current_token) + " instead)").c_str());
			return nullptr;
		}

		std::string name = m_lexer.get_identifier(); // Assuming a getIdentifier() method in the lexer that returns the current identifier
		consume_next_token(); // consume identifier token

		if (m_current_token != token::operator_assignment) {
			ASSERT(false, std::string("[parser]: expected assignment operator (got " + token_to_string(m_current_token) + " instead)").c_str());
			return nullptr;
		}

		consume_next_token();
		node* value = parse_expression();

		if (is_declaration) {
			return new declaration_node(name, value);
		}

		return new assignment_node(name, value);
	}

	node* parser::parse_expression() {
		node* term = parse_term();

		while (m_current_token == token::operator_addition || m_current_token == token::operator_subtraction) {
			token op = m_current_token;
			consume_next_token();

			node* right = parse_term();

			if (op == token::operator_addition) {
				term = new operator_addition_node(term, right);
			}
			else {
				term = new operator_subtraction_node(term, right);
			}
		}

		return term;
	}

	node* parser::parse_term() {
		node* factor = parse_factor();

		while (m_current_token == token::operator_multiplication || m_current_token == token::operator_division) {
			token op = m_current_token;
			consume_next_token();

			node* right = parse_factor();

			if (op == token::operator_multiplication) {
				factor = new operator_multiplication_node(factor, right);
			}
			else {
				factor = new operator_division_node(factor, right);
			}
		}

		return factor;
	}

	node* parser::parse_factor() {
		node* root = nullptr;

		if (m_current_token == token::number_signed) {
			root = parse_number();
		}
		else if (m_current_token == token::identifier) {
			std::string name = m_lexer.get_identifier(); // assuming a getIdentifier() method in the lexer that returns the current identifier
			consume_next_token(); // consume the identifier token

			if (m_current_token == token::l_parenthesis) {
				root = parse_function_call(name);
			}
			else {
				root = new variable_node(name);
			}
		}
		else if (m_current_token == token::l_parenthesis) {
			consume_next_token(); // consume the left parenthesis
			root = parse_expression();
			expect_next_token(token::r_parenthesis); // consume the right parenthesis
		}
		else {
			ASSERT(false, std::string("[parser]: unexpected token received (" + token_to_string(m_current_token) + ")").c_str());
		}

		return root;
	}

	node* parser::parse_number() {
		const i32 value = std::stoi(m_lexer.get_value()); // todo: implement a specific get_number_value() lexer function
		consume_next_token(); // consume the number token
		return new keyword_i32_node(value);
	}

	node* parser::parse_function_call(const std::string& function_name)	{
		consume_next_token(); // consume the left parenthesis

		std::vector<node*> arguments;

		// parse function call arguments
		if (m_current_token != token::r_parenthesis) {
			while (true) {
				node* argument = parse_expression();
				arguments.push_back(argument);

				if (m_current_token == token::comma) {
					consume_next_token(); // Consume the comma
				}
				else if (m_current_token == token::r_parenthesis) {
					break;
				}
				else {
					ASSERT(false, std::string("[parser]: unexpected token received (" + token_to_string(m_current_token) + ")").c_str());
				}
			}
		}

		consume_next_token(); // consume the right parenthesis
		return new function_call_node(function_name, arguments);
	}
}
