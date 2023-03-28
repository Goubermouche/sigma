#pragma once
#include "../utility/macros.h"

namespace language {
	enum class token {
		l_parenthesis,                      // token identifier for the '(' character
		r_parenthesis,                      // token identifier for the ')' character
		l_brace,                            // token identifier for the '{' character
		r_brace,                            // token identifier for the '}' character
					                        
		// keywords			                
		// signed integers	                
		keyword_i8,                         // i8  0
		keyword_i16,                        // i16 0
		keyword_i32,                        // i32 0
		keyword_i64,                        // i64 0
							                
		// unsigned integers                
		keyword_u8,                         // u8  0u
		keyword_u16,                        // u16 0u
		keyword_u32,                        // u32 0u
		keyword_u64,                        // u64 0u
							                
		// floating point	                
		keyword_f32,                        // f32 0.0f
		keyword_f64,                        // f64 0.0
								        
		// operators
		operator_addition,                  // +
		operator_addition_assignment,       // +=
		operator_subtraction,               // -
		operator_subtraction_assignment,    // -=
		operator_multiplication,            // *
		operator_multiplication_assignment, // *=
		operator_modulo,                    // %
		operator_modulo_assignment,         // %=
		operator_division,                  // /
		operator_division_assignment,       // /=
		operator_assignment,                // =
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

		definition,                         // currently only "void"
		identifier,
		semicolon,                          // ;

		end_of_file,
		unknown
	};

	inline std::string token_to_string(const token& token) {
		switch (token) {
		case token::l_brace:
			return "l_brace";
		case token::r_brace:
			return "r_brace";
		case token::l_parenthesis:
			return "l_parenthesis";
		case token::r_parenthesis:
			return "r_parenthesis";

		// keywords
		// signed integers
		case token::keyword_i8:
			return "keyword_i8";
		case token::keyword_i16:
			return "keyword_i16";
		case token::keyword_i32:
			return "keyword_i32";
		case token::keyword_i64:
			return "keyword_i64";

		// unsigned integers
		case token::keyword_u8:
			return "keyword_u8";
		case token::keyword_u16:
			return "keyword_u16";
		case token::keyword_u32:
			return "keyword_u32";
		case token::keyword_u64:
			return "keyword_u64";

		// floating point
		case token::keyword_f32:
			return "keyword_f32";
		case token::keyword_f64:
			return "keyword_f64";

		// operators
		case token::operator_addition:
			return "operator_add";
		case token::operator_addition_assignment:
			return "operator_addition_assignment";
		case token::operator_subtraction:
			return "operator_subtraction";
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
		case token::definition:
			return "definition";
		case token::identifier:
			return "identifier";
		case token::semicolon:
			return "semicolon";
	
		case token::end_of_file:
			return "end_of_file";
		case token::unknown:
			return "unknown";
		}

		return "";
	}
}
