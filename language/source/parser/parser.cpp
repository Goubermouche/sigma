#include "parser.h"

#include "../codegen/abstract_syntax_tree/keywords/assignment_node.h"
#include "../codegen/abstract_syntax_tree/keywords/function_call_node.h"
#include "../codegen/abstract_syntax_tree/keywords/function_node.h"
#include "../codegen/abstract_syntax_tree/keywords/return_node.h"
#include "../codegen/abstract_syntax_tree/keywords/types/floating_point/keyword_f32_node.h"

#include "../codegen/abstract_syntax_tree/keywords/types/signed_int/keyword_i8_node.h"
#include "../codegen/abstract_syntax_tree/keywords/types/signed_int/keyword_i16_node.h"
#include "../codegen/abstract_syntax_tree/keywords/types/signed_int/keyword_i32_node.h"
#include "../codegen/abstract_syntax_tree/keywords/types/signed_int/keyword_i64_node.h"

#include "../codegen/abstract_syntax_tree/keywords/types/unsigned_int/keyword_u8_node.h"
#include "../codegen/abstract_syntax_tree/keywords/types/unsigned_int/keyword_u16_node.h"
#include "../codegen/abstract_syntax_tree/keywords/types/unsigned_int/keyword_u32_node.h"
#include "../codegen/abstract_syntax_tree/keywords/types/unsigned_int/keyword_u64_node.h"

#include "../codegen/abstract_syntax_tree/keywords/types/floating_point/keyword_f32_node.h"
#include "../codegen/abstract_syntax_tree/keywords/types/floating_point/keyword_f64_node.h"

#include "../codegen/abstract_syntax_tree/variables/variable_node.h"
#include "../codegen/abstract_syntax_tree/variables/declaration/local_declaration_node.h"
#include "../codegen/abstract_syntax_tree/variables/declaration/global_declaration_node.h"

#include "../codegen/abstract_syntax_tree/operators/operator_addition_node.h"
#include "../codegen/abstract_syntax_tree/operators/operator_division_node.h"
#include "../codegen/abstract_syntax_tree/operators/operator_multiplication_node.h"
#include "../codegen/abstract_syntax_tree/operators/operator_subtraction_node.h"
#include "../codegen/abstract_syntax_tree/operators/operator_modulo_node.h"

namespace channel {
	parser::parser(const std::string& source_file)
		: m_lexer(source_file) {
	}

	std::vector<node*> parser::parse() {
		std::vector<node*> program;

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

		if(is_global) {
			switch (m_current_token) {
			case token::identifier:
				statement = parse_assignment(is_global);
				break;
			case token::keyword_type_i8:
			case token::keyword_type_i16:
			case token::keyword_type_i32:
			case token::keyword_type_i64:
			case token::keyword_type_u8:
			case token::keyword_type_u16:
			case token::keyword_type_u32:
			case token::keyword_type_u64:
			case token::keyword_type_f32:
			case token::keyword_type_f64:
				statement = parse_declaration(is_global, m_current_token);
				break;
			default:
				std::cout << "unhandled token (" << token_to_string(m_current_token) << ") \n";
				break;
			}
		}
		else {
			switch (m_current_token) {
			case token::identifier:
				if(peek_is_function_call()) {
					statement = parse_function_call(m_lexer.get_identifier());
				}
				else {
					statement = parse_assignment(is_global);
				}

				break;
			case token::keyword_type_i8:
			case token::keyword_type_i16:
			case token::keyword_type_i32:
			case token::keyword_type_i64:
			case token::keyword_type_u8:
			case token::keyword_type_u16:
			case token::keyword_type_u32:
			case token::keyword_type_u64:
			case token::keyword_type_f32:
			case token::keyword_type_f64:
				statement = parse_declaration(is_global, m_current_token);
				break;
			case token::keyword_return:
				statement = parse_return_statement();
				break;
			default:
				std::cout << "unhandled token (" << token_to_string(m_current_token) << ") \n";
				break;
			}
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

	node* parser::parse_declaration(bool is_global, token type_token) {
		consume_next_token();

		ASSERT(m_current_token == token::identifier, "[parser]: expected an identifier, but received '" + token_to_string(m_current_token) + "' instead");
		const std::string name = m_lexer.get_identifier();
		consume_next_token(); // consume identifier token

		node* value = nullptr;

		if (m_current_token == token::operator_assignment) {
			consume_next_token();
			value = parse_expression(type_token);
		}

		if (is_global) {
			return new global_declaration_node(m_lexer.get_current_line_index(), token_to_type(type_token), name, value);
		}

		return new local_declaration_node(m_lexer.get_current_line_index(), token_to_type(type_token), name, value);
	}

	node* parser::parse_assignment(bool is_global) {
		const std::string name = m_lexer.get_identifier();
		consume_next_token(); // = 
		// parse access-assignment 
		ASSERT(m_current_token == token::operator_assignment, "[parser]: expected an assignment operator, but received '" + token_to_string(m_current_token) + "' instead");
		node* value;
		consume_next_token(); // value

		if (peek_is_function_call()) {
			// parse function call and assign the result to the variable
			const std::string function_name = m_lexer.get_identifier();
			value = parse_function_call(function_name);
		}
		else {
			value = parse_expression();
		}

		return new assignment_node(m_lexer.get_current_line_index(), name, value);
	}

	node* parser::parse_expression(token type_token) {
		node* term = parse_term(type_token);

		while (m_current_token == token::operator_addition || m_current_token == token::operator_subtraction) {
			const token op = m_current_token;
			consume_next_token();

			node* right = parse_term(type_token);

			if (op == token::operator_addition) {
				term = new operator_addition_node(m_lexer.get_current_line_index(), term, right);
			}
			else {
				term = new operator_subtraction_node(m_lexer.get_current_line_index(), term, right);
			}
		}

		return term;
	}

	node* parser::parse_term(token type_token) {
		node* factor = parse_factor(type_token);

		while (m_current_token == token::operator_multiplication || m_current_token == token::operator_division || m_current_token == token::operator_modulo) {
			const token op = m_current_token;
			consume_next_token();
			node* right = parse_factor(type_token);

			if(op == token::operator_multiplication) {
				factor = new operator_multiplication_node(m_lexer.get_current_line_index(), factor, right);
			}
			else if(op == token::operator_division) {
				factor = new operator_division_node(m_lexer.get_current_line_index(), factor, right);
			}
			else if(op == token::operator_modulo) {
				factor = new operator_modulo_node(m_lexer.get_current_line_index(), factor, right);
			}
		}

		return factor;
	}

	node* parser::parse_factor(token type_token) {
		node* root = nullptr;

		if (is_token_numerical(m_current_token)) {
			root = parse_number(type_token);
		}
		else if (m_current_token == token::operator_subtraction) {
			consume_next_token(); // consume the subtraction token

			// negate the number by subtracting it from 0
			node* zero_node = create_zero_node(token_to_type(type_token));
			root = new operator_subtraction_node(m_lexer.get_current_line_index(), zero_node, parse_number(type_token));
		}
		else if (m_current_token == token::identifier) {
			const std::string name = m_lexer.get_identifier();
			consume_next_token(); // consume the identifier token

			if (m_current_token == token::l_parenthesis) {
				root = parse_function_call(name);
			}
			else {
				root = new variable_node(m_lexer.get_current_line_index(), name);
			}
		}
		else if (m_current_token == token::l_parenthesis) {
			consume_next_token(); // consume the left parenthesis
			root = parse_expression(type_token);
			expect_next_token(token::r_parenthesis); // consume the right parenthesis
		}
		else {
			ASSERT(false, "[parser]: received an unexpected token '" + token_to_string(m_current_token) + "'");
		}

		return root;
	}

	node* parser::parse_number(token type_token) {
		const std::string str_value = m_lexer.get_value();
		const token type = type_token != token::unknown ? type_token : m_current_token;
		consume_next_token();

		switch (type) {
			// signed
			case token::keyword_type_i8:  return new keyword_i8_node(m_lexer.get_current_line_index(), std::stoll(str_value));
			case token::keyword_type_i16: return new keyword_i16_node(m_lexer.get_current_line_index(), std::stoll(str_value));
			case token::number_signed:
			case token::keyword_type_i32: return new keyword_i32_node(m_lexer.get_current_line_index(),std::stoll(str_value));
			case token::keyword_type_i64: return new keyword_i64_node(m_lexer.get_current_line_index(),std::stoll(str_value));
			// unsigned
			case token::keyword_type_u8:  return new keyword_u8_node(m_lexer.get_current_line_index(), std::stoull(str_value));
			case token::keyword_type_u16: return new keyword_u16_node(m_lexer.get_current_line_index(), std::stoull(str_value));
			case token::number_unsigned:
			case token::keyword_type_u32: return new keyword_u32_node(m_lexer.get_current_line_index(),std::stoull(str_value));
			case token::keyword_type_u64: return new keyword_u64_node(m_lexer.get_current_line_index(),std::stoull(str_value));
			// floating point
			case token::number_f32:
			case token::keyword_type_f32: return new keyword_f32_node(m_lexer.get_current_line_index(), std::stof(str_value));
			case token::number_f64:
			case token::keyword_type_f64: return new keyword_f64_node(m_lexer.get_current_line_index(), std::stod(str_value));
		default:
			ASSERT(false, "[parser]: unhandled number format '" + token_to_string(type) + "' encountered");
			return nullptr;
		}
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
		return new function_call_node(m_lexer.get_current_line_index(), function_name, arguments);
	}

	node* parser::parse_function_definition() {
		const type return_type = token_to_type(m_current_token);

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
		return new function_node(m_lexer.get_current_line_index(), return_type, name, std::move(statements));
	}

	node* parser::parse_return_statement() {
		consume_next_token();
		node* expression = parse_expression();
		return new return_node(m_lexer.get_current_line_index(), expression);
	}

	node* parser::create_zero_node(type ty) const {
		switch (ty) {
			case type::i8:  return new keyword_i8_node(m_lexer.get_current_line_index(), 0);
			case type::i16: return new keyword_i16_node(m_lexer.get_current_line_index(), 0);
			case type::i32: return new keyword_i32_node(m_lexer.get_current_line_index(), 0);
			case type::i64: return new keyword_i64_node(m_lexer.get_current_line_index(), 0);
			case type::u8:  return new keyword_u8_node(m_lexer.get_current_line_index(), 0);
			case type::u16: return new keyword_u16_node(m_lexer.get_current_line_index(), 0);
			case type::u32: return new keyword_u32_node(m_lexer.get_current_line_index(), 0);
			case type::u64: return new keyword_u64_node(m_lexer.get_current_line_index(), 0);
			case type::f32: return new keyword_f32_node(m_lexer.get_current_line_index(), 0.0);
			case type::f64: return new keyword_f64_node(m_lexer.get_current_line_index(), 0.0);
			default:
				ASSERT(false, "[parser]: cannot convert '" + type_to_string(ty) + "' to a type keyword");
				return nullptr;
		}
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

	bool parser::peek_is_function_definition() const {
		// save the current state
		lexer temp_lexer = m_lexer;
		temp_lexer.get_token(); // identifier
		const token tok = temp_lexer.get_token(); // (
		return tok == token::l_parenthesis;
	}

	bool parser::peek_is_function_call() const {
		// save the current state
		lexer temp_lexer = m_lexer;
		temp_lexer.get_token(); // identifier
		const token tok = temp_lexer.get_token(); // (
		return tok == token::l_parenthesis;
	}
}
