#include "parser.h"
// functions
#include "../codegen/abstract_syntax_tree/functions/function_call_node.h"
#include "../codegen/abstract_syntax_tree/functions/function_node.h"

// variables
#include "../codegen/abstract_syntax_tree/variables/assignment_node.h"
#include "../codegen/abstract_syntax_tree/variables/variable_node.h"
#include "../codegen/abstract_syntax_tree/variables/declaration/local_declaration_node.h"
#include "../codegen/abstract_syntax_tree/variables/declaration/global_declaration_node.h"

// flow control
#include "../codegen/abstract_syntax_tree/keywords/flow_control/return_node.h"

// types
// signed integers
#include "../codegen/abstract_syntax_tree/keywords/types/signed_int/i8_node.h"
#include "../codegen/abstract_syntax_tree/keywords/types/signed_int/i16_node.h"
#include "../codegen/abstract_syntax_tree/keywords/types/signed_int/i32_node.h"
#include "../codegen/abstract_syntax_tree/keywords/types/signed_int/i64_node.h"
// unsigned integers
#include "../codegen/abstract_syntax_tree/keywords/types/unsigned_int/u8_node.h"
#include "../codegen/abstract_syntax_tree/keywords/types/unsigned_int/u16_node.h"
#include "../codegen/abstract_syntax_tree/keywords/types/unsigned_int/u32_node.h"
#include "../codegen/abstract_syntax_tree/keywords/types/unsigned_int/u64_node.h"
// floating point
#include "../codegen/abstract_syntax_tree/keywords/types/floating_point/f32_node.h"
#include "../codegen/abstract_syntax_tree/keywords/types/floating_point/f64_node.h"

// operators
#include "../codegen/abstract_syntax_tree/operators/operator_addition_node.h"
#include "../codegen/abstract_syntax_tree/operators/operator_division_node.h"
#include "../codegen/abstract_syntax_tree/operators/operator_multiplication_node.h"
#include "../codegen/abstract_syntax_tree/operators/operator_subtraction_node.h"
#include "../codegen/abstract_syntax_tree/operators/operator_modulo_node.h"

namespace channel {
	parser::parser(const std::string& source_file)
		: m_lexer(source_file) {}

	bool parser::parse(std::vector<node*>& abstract_syntax_tree) {
		consume_next_token();

		// parse individual statements 
		while (m_current_token != token::end_of_file) {
			node* node;
			if(is_token_return_type(m_current_token) && peek_is_function_definition()) {
				// parse a top-level function definition
				if(!parse_function_definition(node)) {
					return false; // return on failure
				}
			}
			else {
				// parse a top-level statement
				if(!parse_global_statement(node)) {
					return false; // return on failure
				}
			}

			ASSERT(node, "node is null!");
			abstract_syntax_tree.push_back(node);
		}

		return true;
	}

	bool parser::parse_global_statement(node*& out_node) {
		switch (m_current_token) {
		case token::identifier:
			// statements beginning with an identifier has to be an assignment operation
			if (!parse_assignment(out_node, true)) {
				return false; // return on failure
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
			// statements beginning with a type keyword have to be variable or function declarations
			if (!parse_declaration(out_node, true, m_current_token)) {
				return false; // return on failure
			}
			break;
		default:
			compilation_logger::emit_unhandled_token_error(m_lexer.get_current_line_number(), m_current_token);
			return false; // return on failure
		}

		// end statements with a semicolon
		if (!expect_next_token(token::semicolon)) {
			return false; // return on failure
		}

		return true;
	}

	bool parser::parse_local_statement(node*& out_node)	{
		switch (m_current_token) {
		case token::identifier:
			// check if the statement is a function call
			if (peek_is_function_call()) {
				// parse it as a function call
				if (!parse_function_call(out_node, m_lexer.get_identifier())) {
					return false; // return on failure
				}
			}
			// otherwise the statement has to be an assignment
			else {
				if (!parse_assignment(out_node, false)) {
					return false; // return on failure
				}
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
			// statements beginning with a type keyword have to be variable or function declarations
			if (!parse_declaration(out_node, false, m_current_token)) {
				return false; // return on failure
			}
			break;
		case token::keyword_return:
			// parse the 'return' keyword
			if (!parse_return_statement(out_node)) {
				return false; // return on failure
			}
			break;
		default:
			compilation_logger::emit_unhandled_token_error(m_lexer.get_current_line_number(), m_current_token);
			return false; // return on failure
		}

		// end statements with a semicolon
		if (!expect_next_token(token::semicolon)) {
			return false; // return on failure
		}

		return true;
	}

	void parser::consume_next_token() {
		m_current_token = m_lexer.get_token();
	}

	bool parser::expect_next_token(token token)	{
		if(m_current_token == token) {
			consume_next_token();
			return true;
		}

		compilation_logger::emit_unexpected_token_error(m_lexer.get_current_line_number(), token, m_current_token);
		return false; // return on failure
	}

	bool parser::parse_declaration(node*& out_node, bool is_global, token type_token) {
		consume_next_token();

		const std::string name = m_lexer.get_identifier();
		if(!expect_next_token(token::identifier)) {
			return false; // return on failure
		}

		node* value = nullptr;
		if (m_current_token == token::operator_assignment) {
			consume_next_token();
			if(!parse_expression(value, type_token)) {
				return false; // return on failure
			}
		}

		if (is_global) {
			out_node = new global_declaration_node(m_lexer.get_current_line_number(), token_to_type(type_token), name, value);
			return true;
		}

		out_node = new local_declaration_node(m_lexer.get_current_line_number(), token_to_type(type_token), name, value);
		return true;
	}

	bool parser::parse_assignment(node*& out_node, bool is_global) {
		const std::string name = m_lexer.get_identifier();
		consume_next_token(); // =

		if(!expect_next_token(token::operator_assignment)) {
			return false; // return on failure
		}

		// parse access-assignment
		node* value;
		if (peek_is_function_call()) {
			// parse function call and assign the result to the variable
			const std::string function_name = m_lexer.get_identifier();
			 if(!parse_function_call(value, function_name)) {
				 return false; // return on failure
			 }
		}
		else {
			 if(!parse_expression(value)) {
				 return false; // return on failure
			 }
		}

		out_node = new assignment_node(m_lexer.get_current_line_number(), name, value);
		return true;
	}

	bool parser::parse_expression(node*& out_node, token type_token) {
		if(!parse_term(out_node, type_token)) {
			return false; // return on failure
		}

		while (m_current_token == token::operator_addition || m_current_token == token::operator_subtraction) {
			const token op = m_current_token;
			consume_next_token();

			node* right;
			if(!parse_term(right, type_token)) {
				return false; // return on failure
			}

			if (op == token::operator_addition) {
				out_node = new operator_addition_node(m_lexer.get_current_line_number(), out_node, right);
			}
			else {
				out_node = new operator_subtraction_node(m_lexer.get_current_line_number(), out_node, right);
			}
		}

		return true;
	}

	bool parser::parse_term(node*& out_node, token type_token) {
		if(!parse_factor(out_node, type_token)) {
			return false; // return on failure
		}

		while (m_current_token == token::operator_multiplication || m_current_token == token::operator_division || m_current_token == token::operator_modulo) {
			const token op = m_current_token;
			consume_next_token();
			node* right;

			if(!parse_factor(right, type_token)) {
				return false; // return on failure
			}

			if(op == token::operator_multiplication) {
				out_node = new operator_multiplication_node(m_lexer.get_current_line_number(), out_node, right);
			}
			else if(op == token::operator_division) {
				out_node = new operator_division_node(m_lexer.get_current_line_number(), out_node, right);
			}
			else if(op == token::operator_modulo) {
				out_node = new operator_modulo_node(m_lexer.get_current_line_number(), out_node, right);
			}
		}

		return true;
	}

	bool parser::parse_factor(node*& out_node, token type_token) {
		if (is_token_numerical(m_current_token)) {
			if(!parse_number(out_node, type_token)) {
				return false; // return on failure
			}
		}
		else if (m_current_token == token::operator_subtraction) {
			consume_next_token(); // consume the subtraction token

			// negate the number by subtracting it from 0
			node* zero_node = create_zero_node(token_to_type(type_token));
			node* number;

			if(!parse_number(number, type_token)) {
				return false; // return on failure
			}

			out_node = new operator_subtraction_node(m_lexer.get_current_line_number(), zero_node, number);
		}
		else if (m_current_token == token::identifier) {
			const std::string name = m_lexer.get_identifier();
			consume_next_token(); // consume the identifier token

			if (m_current_token == token::l_parenthesis) {
				if(!parse_function_call(out_node, name)) {
					return false; // return on failure
				}
			}
			else {
				out_node = new variable_node(m_lexer.get_current_line_number(), name);
			}
		}
		else if (m_current_token == token::l_parenthesis) {
			consume_next_token(); // consume the left parenthesis

			if(!parse_expression(out_node, type_token)) {
				return false; // return on failure
			}

			expect_next_token(token::r_parenthesis); // consume the right parenthesis
		}
		else {
			compilation_logger::emit_unhandled_token_error(m_lexer.get_current_line_number(), m_current_token);
			return false; // return on failure
		}

		return true;
	}

	bool parser::parse_number(node*& out_node, token type_token) {
		const std::string str_value = m_lexer.get_value();
		const token type = type_token != token::unknown ? type_token : m_current_token;
		consume_next_token();

		switch (type) {
		// signed
		case token::keyword_type_i8:  out_node = new i8_node(m_lexer.get_current_line_number(), std::stoll(str_value)); return true;
		case token::keyword_type_i16: out_node = new i16_node(m_lexer.get_current_line_number(), std::stoll(str_value)); return true;
		case token::number_signed:
		case token::keyword_type_i32: out_node = new i32_node(m_lexer.get_current_line_number(), std::stoll(str_value)); return true;
		case token::keyword_type_i64: out_node = new i64_node(m_lexer.get_current_line_number(), std::stoll(str_value)); return true;
			// unsigned
		case token::keyword_type_u8:  out_node = new u8_node(m_lexer.get_current_line_number(), std::stoull(str_value)); return true;
		case token::keyword_type_u16: out_node = new u16_node(m_lexer.get_current_line_number(), std::stoull(str_value)); return true;
		case token::number_unsigned:
		case token::keyword_type_u32: out_node = new u32_node(m_lexer.get_current_line_number(), std::stoull(str_value)); return true;
		case token::keyword_type_u64: out_node = new u64_node(m_lexer.get_current_line_number(), std::stoull(str_value)); return true;
		// floating point
		case token::number_f32:
		case token::keyword_type_f32: out_node = new f32_node(m_lexer.get_current_line_number(), std::stof(str_value)); return true;
		case token::number_f64:
		case token::keyword_type_f64: out_node = new f64_node(m_lexer.get_current_line_number(), std::stod(str_value)); return true;
		default:
			compilation_logger::emit_unhandled_number_format_error(m_lexer.get_current_line_number(), type);
			return false; // return on failure
		}
	}

	bool parser::parse_function_call(node*& out_node, const std::string& function_name)	{
		consume_next_token(); // consume the left parenthesis

		std::vector<node*> arguments;

		// parse function call arguments
		if (m_current_token != token::r_parenthesis) {
			while (true) {
				node* argument;
				if(!parse_expression(argument)) {
					return false; // return on failure
				}

				arguments.push_back(argument);

				if (m_current_token == token::comma) {
					consume_next_token(); // consume the comma
				}
				else if (m_current_token == token::r_parenthesis) {
					break;
				}
				else {
					compilation_logger::emit_unhandled_token_error(m_lexer.get_current_line_number(), m_current_token);
					return false; // return on failure
				}
			}
		}

		consume_next_token(); // consume the right parenthesis
		out_node = new function_call_node(m_lexer.get_current_line_number(), function_name, arguments);
		return true;
	}

	bool parser::parse_function_definition(node*& out_node) {
		const type return_type = token_to_type(m_current_token);
		const u64 line_number = m_lexer.get_current_line_number();

		consume_next_token();
		// parse the function name (e.g., main, other_function)
		const std::string name = m_lexer.get_identifier();
		consume_next_token();

		// parse the parameter list (assume no parameters for now)
		// todo: add support for parameters
		if (!expect_next_token(token::l_parenthesis) || !expect_next_token(token::r_parenthesis)) {
			return false; // return on failure
		}

		// parse the opening curly brace '{'
		if (!expect_next_token(token::l_brace)) {
			return false; // return on failure
		}

		// parse statements inside the function
		std::vector<node*> statements;
		while (m_current_token != token::r_brace) {
			node* statement;

			if (!parse_local_statement(statement)) {
				return false; // return on failure
			}

			statements.push_back(statement);
		}

		// consume the closing curly brace '}'
		consume_next_token();
		out_node = new function_node(line_number, return_type, name, std::move(statements));
		return true;
	}

	bool parser::parse_return_statement(node*& out_node) {
		consume_next_token();

		node* expression;
		if(!parse_expression(expression)) {
			return false; // return on failure
		}

		out_node = new return_node(m_lexer.get_current_line_number(), expression);
		return true;
	}

	node* parser::create_zero_node(type ty) const {
		switch (ty) {
			case type::i8:  return new i8_node(m_lexer.get_current_line_number(), 0);
			case type::i16: return new i16_node(m_lexer.get_current_line_number(), 0);
			case type::i32: return new i32_node(m_lexer.get_current_line_number(), 0);
			case type::i64: return new i64_node(m_lexer.get_current_line_number(), 0);
			case type::u8:  return new u8_node(m_lexer.get_current_line_number(), 0);
			case type::u16: return new u16_node(m_lexer.get_current_line_number(), 0);
			case type::u32: return new u32_node(m_lexer.get_current_line_number(), 0);
			case type::u64: return new u64_node(m_lexer.get_current_line_number(), 0);
			case type::f32: return new f32_node(m_lexer.get_current_line_number(), 0.0);
			case type::f64: return new f64_node(m_lexer.get_current_line_number(), 0.0);
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
				return false; // return on failure
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
