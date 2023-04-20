#include "parser.h"
// functions
#include "../codegen/abstract_syntax_tree/functions/function_call_node.h"
#include "../codegen/abstract_syntax_tree/functions/function_node.h"

// variables
#include "../codegen/abstract_syntax_tree/variables/assignment_node.h"
#include "../codegen/abstract_syntax_tree/variables/access_node.h"
#include "../codegen/abstract_syntax_tree/variables/array/array_allocation_node.h"
#include "../codegen/abstract_syntax_tree/variables/array/array_access_node.h"
#include "../codegen/abstract_syntax_tree/variables/array/array_assignment_node.h"
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
// text
#include "../codegen/abstract_syntax_tree/keywords/types/text/char_node.h"
#include "../codegen/abstract_syntax_tree/keywords/types/text/string_node.h"

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
		while(true) {
			if(peek_next_token(m_lexer) == token::end_of_file) {
				return true;
			}

			node* node;

			if(peek_is_function_definition(m_lexer)) {
				// parse a top-level function definition
				if(!parse_function_definition(node)) {
					return false;
				}
			}
			else {
				// parse a global statement
				if(!parse_global_statement(node)) {
					return false;
				}
			}

			abstract_syntax_tree.push_back(node);
		}
	}

	void parser::get_next_token() {
		m_current_token = m_lexer.get_token();
	}

	bool parser::expect_next_token(token token) {
		get_next_token();

		if (m_current_token == token) {
			return true;
		}

		compilation_logger::emit_unexpected_token_error(m_lexer.get_current_line_number(), token, m_current_token);
		return false; // return on failure
	}

	bool parser::parse_function_definition(node*& out_node)	{
		const type return_type = parse_type();
		const u64 line_number = m_lexer.get_current_line_number();
		get_next_token(); // identifier (guaranteed)
		const std::string identifier = m_lexer.get_identifier();
		get_next_token(); // l_parenthesis (guaranteed)
		// get_next_token(); // r_parenthesis || type

		std::vector<std::pair<std::string, type>> arguments;

		// parse arguments
		token next_token = peek_next_token(m_lexer);
		if(next_token != token::r_parenthesis) {
			while (true) {
				type argument_type = parse_type();

				if (!expect_next_token(token::identifier)) {
					return false;
				}

				std::string argument_name = m_lexer.get_identifier();
				arguments.emplace_back(argument_name, argument_type);

				// get_next_token(); // comma || type || other
				next_token = peek_next_token(m_lexer);
				if (next_token == token::comma) {
					get_next_token(); // comma (guaranteed)
				}
				else if (next_token != token::r_parenthesis) {
					compilation_logger::emit_unexpected_token_error(m_lexer.get_current_line_number(), token::r_parenthesis, m_current_token);
					return false;
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

		if(!expect_next_token(token::l_brace)) {
			return false;
		}
		
		std::vector<node*> statements;

		next_token = peek_next_token(m_lexer);
		if(next_token != token::r_brace) {
			while (true) {
				if (peek_next_token(m_lexer) == token::r_brace) {
					break;
				}

				node* statement;
				if (!parse_local_statement(statement)) {
					return false;
				}

				statements.push_back(statement);

				next_token = peek_next_token(m_lexer);
				if(next_token == token::r_brace) {
					break;
				}
			}
		}
		
		if (!expect_next_token(token::r_brace)) {
			return false;
		}

		out_node = new function_node(line_number, return_type, identifier, statements, arguments);
		return true;
	}

	bool parser::parse_global_statement(node*& out_node) {
		const token token = peek_next_token(m_lexer); // identifier || type || keyword

		if (is_token_type(token)) {
			// statements beginning with a type keyword have to be variable declarations
			if (!parse_declaration(out_node, true)) {
				return false;
			}
		}
		else {
			switch (token) {
			case token::identifier:
				// assignment statement
				if (!parse_assignment(out_node)) {
					return false;
				}
				break;
			default:
				compilation_logger::emit_unhandled_token_error(m_lexer.get_current_line_number(), token);
				return false; // return on failure
			}
		}

		if (!expect_next_token(token::semicolon)) {
			return false;
		}

		return true;
	}

	bool parser::parse_local_statement(node*& out_node)	{
		const token token = peek_next_token(m_lexer); // identifier || type || keyword

		if(is_token_type(token)) {
			// statements beginning with a type keyword have to be variable declarations
			if(!parse_declaration(out_node, false)) {
				return false;
			}
		}
		else {
			switch (token) {
			case token::identifier:
				if(peek_is_function_call(m_lexer)) {
					// function call statement
					if(!parse_function_call(out_node)) {
						return false;
					}
				}
				else if(peek_is_array_index_access(m_lexer)) {
					// array assignment
					if(!parse_array_assignment(out_node)) {
						return false;
					}
				}
				else {
					// assignment statement
					if (!parse_assignment(out_node)) {
						return false;
					}
				}


				break;
			case token::keyword_return:
				if (!parse_return_statement(out_node)) {
					return false;
				}
				break;
			default:
				compilation_logger::emit_unhandled_token_error(m_lexer.get_current_line_number(), token);
				return false; // return on failure
			}
		}

		if(!expect_next_token(token::semicolon)) {
			return false;
		}

		return true;
	}

	bool parser::parse_array_assignment(node*& out_node) {
		get_next_token(); // identifier (guaranteed)
		const std::string identifier = m_lexer.get_identifier();
		get_next_token(); // l_bracket (guaranteed)

		node* index_node;
		if (!parse_expression(index_node)) {
			return false;
		}

		// Make sure the next token is a right square bracket
		if (!expect_next_token(token::r_bracket)) {
			return false;
		}

		if (!expect_next_token(token::operator_assignment)) {
			return false;
		}

		// parse access-assignment
		node* value;
		if (peek_is_function_call(m_lexer)) {
			if (!parse_function_call(value)) {
				return false;
			}
		}
		else {
			if (!parse_expression(value)) {
				return false;
			}
		}

		out_node = new array_assignment_node(m_lexer.get_current_line_number(), identifier, index_node, value);
		return true;
	}

	bool parser::parse_assignment(node*& out_node) {
		get_next_token(); // identifier (guaranteed)
		const std::string identifier = m_lexer.get_identifier();

		if (!expect_next_token(token::operator_assignment)) {
			return false; 
		}

		// parse access-assignment
		node* value;
		if (peek_is_function_call(m_lexer)) {
			if (!parse_function_call(value)) {
				return false;
			}
		}
		else {
			if (!parse_expression(value)) {
				return false;
			}
		}

		out_node = new assignment_node(m_lexer.get_current_line_number(), identifier, value);
		return true;
	}

	bool parser::parse_array_access(node*& out_node) {
		get_next_token(); // identifier (guaranteed)
		const std::string identifier = m_lexer.get_identifier();

		get_next_token(); // l_bracket

		// parse access index
		node* array_index;
		if (!parse_expression(array_index, type::u64)) {
			return false;
		}

		get_next_token(); // r_bracket

		out_node = new array_access_node(m_lexer.get_current_line_number(), identifier, array_index);
		return true;
	}

	bool parser::parse_function_call(node*& out_node) {
		get_next_token(); // identifier (guaranteed)
		const std::string identifier = m_lexer.get_identifier();

		get_next_token(); // l_parenthesis (guaranteed)

		std::vector<node*> arguments;

		token next_token = peek_next_token(m_lexer);
		if(next_token != token::r_parenthesis) {
			while(true) {
				node* argument;
				if (!parse_expression(argument)) {
					return false;
				}

				arguments.push_back(argument);

				next_token = peek_next_token(m_lexer); // comma || r_parenthesis || other

				if (next_token == token::comma) {
					get_next_token(); // comma (guaranteed)
				}
				else if (next_token == token::r_parenthesis) {
					break;
				}
				else {
					compilation_logger::emit_unhandled_token_error(m_lexer.get_current_line_number(), m_current_token);
					return false; 
				}
			}
		}

		if(!expect_next_token(token::r_parenthesis)) {
			return false;
		}

		out_node = new function_call_node(m_lexer.get_current_line_number(), identifier, arguments);
		return true;
	}

	bool parser::parse_return_statement(node*& out_node) {
		get_next_token(); // keyword_return (guaranteed)

		node* expression;
		if(!parse_expression(expression)) {
			return false;
		}

		out_node = new return_node(m_lexer.get_current_line_number(), expression);
		return true;
	}

	bool parser::parse_declaration(node*& out_node, bool is_global) {
		const type declaration_type = parse_type();

		if(!expect_next_token(token::identifier)) {
			return false;
		}

		const std::string identifier = m_lexer.get_identifier();

		node* value = nullptr;
		if(peek_next_token(m_lexer) == token::operator_assignment) {
			get_next_token(); // operator_assignment
			if(!parse_expression(value, declaration_type)) {
				return false;
			}
		}

		if(is_global) {
			out_node = new global_declaration_node(m_lexer.get_current_line_number(), declaration_type, identifier, value);
			return true;
		}

		out_node = new local_declaration_node(m_lexer.get_current_line_number(), declaration_type, identifier, value);
		return true;
	}

	bool parser::parse_expression(node*& out_node, type expression_type) {
		if(!parse_term(out_node, expression_type)) {
			return false;
		}

		token next_token = peek_next_token(m_lexer);
		if (next_token == token::operator_addition || next_token == token::operator_subtraction) {
			while(true) {
				const token op = next_token;
				node* right;

				get_next_token(); // operator_addition || operator_subtraction (guaranteed)

				if (!parse_term(right, expression_type)) {
					return false;
				}

				if (op == token::operator_addition) {
					out_node = new operator_addition_node(m_lexer.get_current_line_number(), out_node, right);
				}
				else {
					out_node = new operator_subtraction_node(m_lexer.get_current_line_number(), out_node, right);
				}

				next_token = peek_next_token(m_lexer);
				if (!(next_token == token::operator_addition || next_token == token::operator_subtraction || next_token == token::operator_modulo)) {
					break;
				}
			}
		}

		return true;
	}

	bool parser::parse_term(node*& out_node, type expression_type) {
		if(!parse_factor(out_node, expression_type)) {
			return false;
		}

		token next_token = peek_next_token(m_lexer); // operator_multiplication || operator_division || operator_modulo
		if(next_token == token::operator_multiplication || next_token == token::operator_division || next_token == token::operator_modulo) {
			while (true) {
				const token op = next_token;
				node* right;

				get_next_token(); // operator_multiplication || operator_division || operator_modulo (guaranteed)

				if (!parse_factor(right, expression_type)) {
					return false;
				}

				if (op == token::operator_multiplication) {
					out_node = new operator_multiplication_node(m_lexer.get_current_line_number(), out_node, right);
				}
				else if (op == token::operator_division) {
					out_node = new operator_division_node(m_lexer.get_current_line_number(), out_node, right);
				}
				else if (op == token::operator_modulo) {
					out_node = new operator_modulo_node(m_lexer.get_current_line_number(), out_node, right);
				}

				next_token = peek_next_token(m_lexer);
				if (!(next_token == token::operator_multiplication || next_token == token::operator_division || next_token == token::operator_modulo)) {
					break;
				}
			}
		}

		return true;
	}

	bool parser::parse_factor(node*& out_node, type expression_type) {
		const token token = peek_next_token(m_lexer);

		if(is_token_numerical(token)) {
			// parse a number
			return parse_number(out_node, expression_type);
		}

		switch(token) {
		case token::operator_subtraction:
			// parse a negative number
			return parse_negative_number(out_node, expression_type);
		case token::identifier:
			// parse a function call or an assignment
			return parse_function_call_or_assignment(out_node);
		case token::l_parenthesis:
			// parse a deep expression
			return parse_deep_expression(out_node, expression_type);
		case token::keyword_new:
			// parse an allocation
			return parse_new_allocation(out_node);
		case token::char_literal:
			return parse_char(out_node);
		case token::string_literal:
			return parse_string(out_node);
		}

		compilation_logger::emit_unhandled_token_error(m_lexer.get_current_line_number(), m_current_token);
		return false;
	}

	bool parser::parse_number(node*& out_node, type expression_type) {
		get_next_token(); // type
		const std::string str_value = m_lexer.get_value();
		const type type = expression_type == type::unknown ? token_to_type(m_current_token) : expression_type;
		// std::cout << "parsed type: " << type_to_string(type) << '\n';

		switch (type) {
		// signed
		case type::i8:  out_node = new i8_node(m_lexer.get_current_line_number(), std::stoll(str_value)); return true;
		case type::i16: out_node = new i16_node(m_lexer.get_current_line_number(), std::stoll(str_value)); return true;
		case type::i32: out_node = new i32_node(m_lexer.get_current_line_number(), std::stoll(str_value)); return true;
		case type::i64: out_node = new i64_node(m_lexer.get_current_line_number(), std::stoll(str_value)); return true;
			// unsigned
		case type::u8:  out_node = new u8_node(m_lexer.get_current_line_number(), std::stoull(str_value)); return true;
		case type::u16: out_node = new u16_node(m_lexer.get_current_line_number(), std::stoull(str_value)); return true;
		case type::u32: out_node = new u32_node(m_lexer.get_current_line_number(), std::stoull(str_value)); return true;
		case type::u64: out_node = new u64_node(m_lexer.get_current_line_number(), std::stoull(str_value)); return true;
		// floating point
		case type::f32: out_node = new f32_node(m_lexer.get_current_line_number(), std::stof(str_value)); return true;
		case type::f64: out_node = new f64_node(m_lexer.get_current_line_number(), std::stod(str_value)); return true;
		default:
			compilation_logger::emit_unhandled_number_format_error(m_lexer.get_current_line_number(), type);
			return false; // return on failure
		}
	}

	bool parser::parse_char(node*& out_node) {
		get_next_token(); // char_literal (guaranteed)
		out_node = new char_node(m_lexer.get_current_line_number(), m_lexer.get_value()[0]);
		return true;
	}

	bool parser::parse_string(node*& out_node) {
		get_next_token(); // string_literal (guaranteed)
		out_node = new string_node(m_lexer.get_current_line_number(), m_lexer.get_value());
		return true;
	}

	bool parser::parse_negative_number(node*& out_node, type expression_type) {
		get_next_token(); // operator_subtraction (guaranteed)

		// negate the number by subtracting it from 0
		node* zero_node = create_zero_node(expression_type);
		node* number;

		if (!parse_number(number, expression_type)) {
			return false;
		}

		out_node = new operator_subtraction_node(m_lexer.get_current_line_number(), zero_node, number);
		return true;
	}

	bool parser::parse_new_allocation(node*& out_node) {
		get_next_token(); // keyword_new (guaranteed)

		const u64 line_number = m_lexer.get_current_line_number();
		const type allocation_type = parse_type();

		// l_bracket
		if (!expect_next_token(token::l_bracket)) {
			return false;
		}

		// parse array size
		node* array_size;
		if(!parse_expression(array_size, type::u64)) {
			return false;
		}

		// r_bracket
		if (!expect_next_token(token::r_bracket)) {
			return false;
		}

		out_node = new array_allocation_node(line_number, allocation_type, array_size);
		return true;
	}

	bool parser::parse_function_call_or_assignment(node*& out_node) {
		if (peek_is_function_call(m_lexer)) {
			// parse a function call
			return parse_function_call(out_node);
		}

		if(peek_is_array_index_access(m_lexer)) {
			return parse_array_access(out_node);
		}

		// parse an assignment
		get_next_token();
		const std::string identifier = m_lexer.get_identifier();
		out_node = new access_node(m_lexer.get_current_line_number(), identifier);

		return true;
	}

	bool parser::parse_deep_expression(node*& out_node, type expression_type) {
		get_next_token(); // l_parenthesis (guaranteed)

		// nested expression
		if (!parse_expression(out_node, expression_type)) {
			return false;
		}

		// r_parenthesis
		if (!expect_next_token(token::r_parenthesis)) {
			return false;
		}

		return true;
	}
	
	bool parser::peek_is_function_definition(lexer lexer_copy) {
		// type
		if(!is_token_type(lexer_copy.get_token())) {
			return false;
		}

		// type
		const token next_token = peek_next_token(lexer_copy);
		if(next_token == token::operator_multiplication) {
			lexer_copy.get_token(); // operator_multiplication (guaranteed)
		}
		else if(next_token != token::identifier) {
			return false;
		}

		// identifier
		if(lexer_copy.get_token() != token::identifier) {
			return false;
		}

		// l_parenthesis
		return lexer_copy.get_token() == token::l_parenthesis;
	}

	bool parser::peek_is_function_call(lexer lexer_copy) {
		// identifier
		if(lexer_copy.get_token() != token::identifier) {
			return false;
		}

		return lexer_copy.get_token() == token::l_parenthesis;
	}

	bool parser::peek_is_array_index_access(lexer lexer_copy) {
		// identifier
		if (lexer_copy.get_token() != token::identifier) {
			return false;
		}

		return lexer_copy.get_token() == token::l_bracket;
	}

	token parser::peek_next_token(lexer lexer_copy) {
		return lexer_copy.get_token();
	}

	node* parser::create_zero_node(type expression_type) const {
		switch (expression_type) {
		case type::i8:  return new i8_node(m_lexer.get_current_line_number(), 0);
		case type::i16: return new i16_node(m_lexer.get_current_line_number(), 0);
		case type::i32: return new i32_node(m_lexer.get_current_line_number(), 0);
		case type::i64: return new i64_node(m_lexer.get_current_line_number(), 0);
		case type::u8:  return new u8_node(m_lexer.get_current_line_number(), 0);
		case type::u16: return new u16_node(m_lexer.get_current_line_number(), 0);
		case type::u32: return new u32_node(m_lexer.get_current_line_number(), 0);
		case type::u64: return new u64_node(m_lexer.get_current_line_number(), 0);
		case type::f32: return new f32_node(m_lexer.get_current_line_number(), 0.0f);
		case type::f64: return new f64_node(m_lexer.get_current_line_number(), 0.0);
		default:
			ASSERT(false, "[parser]: cannot convert '" + type_to_string(expression_type) + "' to a type keyword");
			return nullptr;
		}
	}

	type parser::parse_type() {
		get_next_token();

		if(!is_token_type(m_current_token)) {
			compilation_logger::emit_token_is_not_type_error(m_lexer.get_current_line_number(), m_current_token);
		}

		const type type = token_to_type(m_current_token);

		// check if the next token is an asterisk
		if(peek_next_token(m_lexer) == token::operator_multiplication) {
			get_next_token(); // operator_multiplication (guaranteed)
			// get the respective pointer type
			return get_pointer_type(type);
		}

		return type;
	}
}
