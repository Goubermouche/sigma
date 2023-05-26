#pragma once
#include "utility/macros.h"

namespace channel {
	enum class token {
		l_parenthesis,                      // (
		r_parenthesis,                      // )
		l_brace,                            // {
		r_brace,                            // }
		l_bracket,                          // [
		r_bracket,                          // ]
		comma,                              // ,
		semicolon,                          // ;
		single_quote,                       // '
		double_quote,                       // "

		// keywords
		keyword_type_structure,             // struct
		keyword_type_void,                  // void
		keyword_type_bool,                  // bool
		bool_literal_true,                  // true
		bool_literal_false,                 // false

		// signed integers	                
		keyword_type_i8,                    // i8  0
		keyword_type_i16,                   // i16 0
		keyword_type_i32,                   // i32 0
		keyword_type_i64,                   // i64 0
							                
		// unsigned integers                
		keyword_type_u8,                    // u8  0u
		keyword_type_u16,                   // u16 0u
		keyword_type_u32,                   // u32 0u
		keyword_type_u64,                   // u64 0u
							                
		// floating point	                
		keyword_type_f32,                   // f32 0.0f
		keyword_type_f64,                   // f64 0.0

		// text
		keyword_type_char,                  // char
		char_literal,                       // 'c'
		string_literal,                     // "str"

		// statements
		// flow control
		keyword_return,                     // return
		keyword_if,                         // if
		keyword_else,                       // else
		keyword_while,                      // while
		keyword_for,                        // for
		keyword_break,                      // break

		keyword_new,                        // new

		// math
		// vectors
		// vec2<T>
		// vec3<T> 
		// vec4<T>
		// matrices
		// tensors (?)

		// containers
		// T[n]
		// array<T>
		// list<T>
		// map<K, V>
								        
		// operators
		// arithmetic operators
		operator_addition,                  // +
		operator_increment,                 // ++
		operator_addition_assignment,       // +=
		operator_subtraction,               // -
		operator_decrement,                 // --
		operator_subtraction_assignment,    // -=
		operator_multiplication,            // *
		operator_multiplication_assignment, // *=
		operator_modulo,                    // %
		operator_modulo_assignment,         // %=
		operator_division,                  // /
		operator_division_assignment,       // /=
		operator_assignment,                // =
		// logical operators
		operator_equals,                    // ==
		operator_greater_than,              // >
		operator_bitwise_right_shift,       // >>
		operator_greater_than_equal_to,     // >=
		operator_less_than,                 // <
		operator_bitwise_left_shift,        // <<
		operator_less_than_equal_to,        // <=
		operator_not,                       // !
		operator_not_equals,                // !=
		operator_bitwise_and,               // &
		operator_logical_conjunction,       // &&
		operator_bitwise_or,                // |
		operator_logical_disjunction,       // ||
		operator_bitwise_not,               // ~
		operator_bitwise_xor,               // ^

		// numbers
		number_signed,                      // 0
		number_unsigned,                    // 0u
		number_f32,                         // 0.0f
		number_f64,                         // 0.0

		identifier,

		end_of_file,
		unknown
	};

	struct token_position {
		token_position() = default;
		token_position(const std::string& file, u64 line_number, u64 character_number)
			: m_file(file), m_line_number(line_number), m_character_number(character_number) {}

		const std::string& get_file() const {
			return m_file;
		}

		u64 get_line_number() const {
			return m_line_number;
		}

		u64 get_character_number() const {
			return m_character_number;
		}
	private:
		std::string m_file;
		u64 m_line_number;
		u64 m_character_number;
	};

	inline std::string token_to_string(token token) {
		switch (token) {
		case token::l_brace:
			return "l_brace";
		case token::r_brace:
			return "r_brace";
		case token::l_parenthesis:
			return "l_parenthesis";
		case token::r_parenthesis:
			return "r_parenthesis";
		case token::l_bracket:
			return "l_bracket";
		case token::r_bracket:
			return "r_bracket";
		case token::comma:
			return "comma";
		case token::semicolon:
			return "semicolon";
		case token::single_quote:
			return "single_quote";
		case token::double_quote:
			return "double_quote";

		// keywords
		// types
		case token::keyword_type_structure:
			return "keyword_type_structure";
		case token::keyword_type_void:
			return "keyword_type_void";
		case token::keyword_type_bool:
			return "keyword_type_bool";
		case token::bool_literal_true:
			return "bool_literal_true";
		case token::bool_literal_false:
			return "bool_literal_false";

		// signed integers
		case token::keyword_type_i8:
			return "keyword_type_i8";
		case token::keyword_type_i16:
			return "keyword_type_i16";
		case token::keyword_type_i32:
			return "keyword_type_i32";
		case token::keyword_type_i64:
			return "keyword_type_i64";

		// unsigned integers
		case token::keyword_type_u8:
			return "keyword_type_u8";
		case token::keyword_type_u16:
			return "keyword_type_u16";
		case token::keyword_type_u32:
			return "keyword_type_u32";
		case token::keyword_type_u64:
			return "keyword_type_u64";

		// floating point
		case token::keyword_type_f32:
			return "keyword_type_f32";
		case token::keyword_type_f64:
			return "keyword_type_f64";

		// text
		case token::keyword_type_char:
			return "keyword_type_char";
		case token::char_literal:
			return "char_literal";
		case token::string_literal:
			return "string_literal";

		// flow control
		case token::keyword_return:
			return "keyword_return";
		case token::keyword_if:
			return "keyword_if";
		case token::keyword_else:
			return "keyword_else";
		case token::keyword_while:
			return "keyword_while";
		case token::keyword_for:
			return "keyword_for";
		case token::keyword_break:
			return "keyword_break";

		case token::keyword_new:
			return "keyword_new";

		// operators
		case token::operator_addition:
			return "operator_addition";
		case token::operator_increment:
			return "operator_increment";
		case token::operator_addition_assignment:
			return "operator_addition_assignment";
		case token::operator_subtraction:
			return "operator_subtraction";
		case token::operator_decrement:
			return "operator_decrement";
		case token::operator_subtraction_assignment:
			return "operator_subtraction_assignment";
		case token::operator_multiplication:
			return "operator_multiplication";
		case token::operator_multiplication_assignment:
			return "operator_multiplication_assignment";
		case token::operator_modulo:
			return "operator_modulo";
		case token::operator_modulo_assignment:
			return "operator_modulo_assignment";
		case token::operator_division:
			return "operator_division";
		case token::operator_division_assignment:
			return "operator_division_assignment";
		case token::operator_assignment:
			return "operator_assignment";
		case token::operator_equals:
			return "operator_equals";
		case token::operator_greater_than:
			return "operator_greater_than";
		case token::operator_bitwise_right_shift:
			return "operator_bitwise_right_shift";
		case token::operator_greater_than_equal_to:
			return "operator_greater_than_equal_to";
		case token::operator_less_than:
			return "operator_less_than";
		case token::operator_bitwise_left_shift:
			return "operator_bitwise_left_shift";
		case token::operator_less_than_equal_to:
			return "operator_less_than_equal_to";
		case token::operator_not:
			return "operator_not";
		case token::operator_not_equals:
			return "operator_not_equals";
		case token::operator_bitwise_and:
			return "operator_bitwise_and";
		case token::operator_logical_conjunction:
			return "operator_logical_conjunction";
		case token::operator_bitwise_or:
			return "operator_bitwise_or";
		case token::operator_logical_disjunction:
			return "operator_logical_disjunction";
		case token::operator_bitwise_not:
			return "operator_bitwise_not";
		case token::operator_bitwise_xor:
			return "operator_bitwise_xor";

		// numbers
		case token::number_signed:
			return "number_signed";
		case token::number_unsigned:
			return "number_unsigned";
		case token::number_f32:
			return "number_f32";
		case token::number_f64:
			return "number_f64";

		// other
		case token::identifier:
			return "identifier";
		case token::end_of_file:
			return "end_of_file";
		case token::unknown:
			return "unknown";
		}

		return "";
	}

	inline bool is_token_numerical(token token) {
		return token == token::number_f32    ||
			token == token::keyword_type_f32 ||
			token == token::number_f64       ||
			token == token::keyword_type_f64 ||
			token == token::number_signed    ||
			token == token::keyword_type_i8  ||
			token == token::keyword_type_i16 ||
			token == token::keyword_type_i32 ||
			token == token::keyword_type_i64 ||
			token == token::number_unsigned  ||
			token == token::keyword_type_u8  ||
			token == token::keyword_type_u16 ||
			token == token::keyword_type_u32 ||
			token == token::keyword_type_u64;
	}

	inline bool is_token_compound_op(token token) {
		return token == token::operator_addition_assignment    ||
			token == token::operator_subtraction_assignment    ||
			token == token::operator_multiplication_assignment ||
			token == token::operator_modulo_assignment         ||
			token == token::operator_division_assignment;
	}

	inline bool is_token_block_break(token tok) {
		switch (tok) {
		case token::keyword_return:
		case token::keyword_break:
			return true;
		default:
			return false;
		}
	}
}
