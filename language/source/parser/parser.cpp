#include "parser.h"

#include "../codegen/abstract_syntax_tree/keywords/assignment_node.h"
#include "../codegen/abstract_syntax_tree/keywords/declaration_node.h"
#include "../codegen/abstract_syntax_tree/keywords/function_call_node.h"
#include "../codegen/abstract_syntax_tree/variables/variable_node.h"
#include "../codegen/abstract_syntax_tree/keywords/types/keyword_i32_node.h"
#include "../codegen/abstract_syntax_tree/keywords/function_node.h"

#include "../codegen/abstract_syntax_tree/operators/operator_addition_node.h"
#include "../codegen/abstract_syntax_tree/operators/operator_division_node.h"
#include "../codegen/abstract_syntax_tree/operators/operator_multiplication_node.h"
#include "../codegen/abstract_syntax_tree/operators/operator_subtraction_node.h"
#include "../codegen/abstract_syntax_tree/operators/operator_modulo_node.h"

namespace channel {
	parser::parser(const std::string& source_file)
		: m_lexer(source_file) {
		//token token;

		//while ((token = m_lexer.get_token()) != token::end_of_file) {
		//	std::cout << token_to_string(token);

		//	if (token == token::identifier) {
		//		std::cout << " (" << m_lexer.get_identifier() << ')';
		//	}
		//	else if (token == token::number_signed ||
		//		token == token::number_unsigned ||
		//		token == token::number_f32 ||
		//		token == token::number_f64) {
		//		std::cout << " (" << m_lexer.get_value() << ')';
		//	}

		//	std::cout << '\n';
		//}
	}

	std::vector<node*> parser::parse() {
		std::vector<node*> program;

		std::cout << "-----------------------------\n";
		consume_next_token();
		while (m_current_token != token::end_of_file) {
			if(is_token_return_type(m_current_token) && peek_is_function_definition()) {
				// parse a top-level function definition
				program.push_back(parse_function_definition());
			}
			else {
				// parse a top-level statement
				program.push_back(parse_statement(true));
			}
		}

		return program;
	}

	node* parser::parse_statement(bool is_global) {
		node* statement;

		switch (m_current_token) {
		// todo: generalize
		case token::identifier:
		case token::keyword_type_i32:
			statement = parse_declaration_or_assignment(is_global);
			break;
		default:
			std::cout << "unhandled token (" << token_to_string(m_current_token) << ") \n";
			break;
		}

		expect_next_token(token::semicolon);
		return statement;
	}

	void parser::consume_next_token() {
		m_current_token = m_lexer.get_token();
	}

	void parser::expect_next_token(token token)	{
		if(m_current_token == token) {
			consume_next_token();
		}
		else {
			ASSERT(false, "[parser]: unexpected token received (expected '" + token_to_string(token) + "', but received '" + token_to_string(m_current_token) + "' instead)");
		}
	}

	node* parser::parse_declaration_or_assignment(bool is_global) {
		// todo: generalize
		const bool is_declaration = m_current_token == token::keyword_type_i32;
		consume_next_token();

		// parse declaration
		if(is_declaration) {
			ASSERT(m_current_token == token::identifier, "[parser]: expected an identifier, but received '" + token_to_string(m_current_token) + "' instead");
			const std::string name = m_lexer.get_identifier();
			consume_next_token(); // consume identifier token

			node* value = nullptr;

			if (m_current_token == token::operator_assignment) {
				consume_next_token();
				value = parse_expression();
			}

			return new declaration_node(name, is_global, value);
		}

		// parse assignment
		const std::string name = m_lexer.get_identifier();
		ASSERT(m_current_token == token::operator_assignment, "[parser]: expected an assignment operator, but received '" + token_to_string(m_current_token) + "' instead");
		consume_next_token();
		node* value = parse_expression();

		return new assignment_node(name, value);
	}

	node* parser::parse_expression() {
		node* term = parse_term();

		while (m_current_token == token::operator_addition || m_current_token == token::operator_subtraction) {
			const token op = m_current_token;
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

		while (m_current_token == token::operator_multiplication || m_current_token == token::operator_division || m_current_token == token::operator_modulo) {
			const token op = m_current_token;
			consume_next_token();
			node* right = parse_factor();

			if(op == token::operator_multiplication) {
				factor = new operator_multiplication_node(factor, right);
			}
			else if(op == token::operator_division) {
				factor = new operator_division_node(factor, right);
			}
			else if(op == token::operator_modulo) {
				factor = new operator_modulo_node(factor, right);
			}
		}

		return factor;
	}

	node* parser::parse_factor() {
		node* root = nullptr;

		if (m_current_token == token::number_signed) {
			// todo: generalize
			root = parse_number();
		}
		else if (m_current_token == token::identifier) {
			const std::string name = m_lexer.get_identifier();
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
			ASSERT(false, "[parser]: received an unexpected token '" + token_to_string(m_current_token) + "'");
		}

		return root;
	}

	node* parser::parse_number() {
		// todo: generalize
		const i32 value = std::stoi(m_lexer.get_value());
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
					consume_next_token(); // consume the comma
				}
				else if (m_current_token == token::r_parenthesis) {
					break;
				}
				else {
					ASSERT(false, "[parser]: received an unexpected token '" + token_to_string(m_current_token) + "'");
				}
			}
		}

		consume_next_token(); // consume the right parenthesis
		return new function_call_node(function_name, arguments);
	}

	node* parser::parse_function_definition() {
		// parse the return type (e.g., int)
		// todo: generalize
		const std::string return_type = "int";
		consume_next_token();
		// parse the function name (e.g., main, other_function)
		const std::string name = m_lexer.get_identifier();
		consume_next_token();

		// parse the parameter list (assume no parameters for now)
		// todo: add support for parameters
		expect_next_token(token::l_parenthesis);
		expect_next_token(token::r_parenthesis);

		// parse the opening curly brace '{'
		expect_next_token(token::l_brace);

		// parse statements inside the function
		std::vector<node*> statements;
		while (m_current_token != token::r_brace) {
			statements.push_back(parse_statement(false));
		}

		// consume the closing curly brace '}'
		consume_next_token();

		return new function_node(return_type, name, std::move(statements));
	}

	bool parser::is_token_return_type(token token) {
		switch (token) {
			case token::keyword_type_void:
			// signed integers
			case token::keyword_type_i8:
			case token::keyword_type_i16:
			case token::keyword_type_i32:
			case token::keyword_type_i64:
			// unsigned integers          
			case token::keyword_type_u8:
			case token::keyword_type_u16:
			case token::keyword_type_u32:
			case token::keyword_type_u64:
			// floating point
			case token::keyword_type_f32:
			case token::keyword_type_f64:
				return true;
			default:
				return false;
		}
	}

	bool parser::peek_is_function_definition() {
		// save the current state
		const token saved_token = m_current_token;
		const u64 saved_position = m_lexer.get_position();

		// peek the next tokens
		consume_next_token();
		consume_next_token();

		// check if the current token is an opening parenthesis
		const bool is_function_definition = (m_current_token == token::l_parenthesis);

		// restore the saved state
		m_current_token = saved_token;
		m_lexer.set_position(saved_position);

		return is_function_definition;
	}
}
