#pragma once

#include "token.h"
#include "string_accessor.h"

namespace language {
	class lexer {
	public:
		lexer(const std::string& source_file);

		/**
		 * \brief Extracts the next token from the given source file and returns it.
		 * \return Extracted token
		 */
		token get_token();

		/**
		 * \brief Returns the last known identifier from the lexing process. If there isn't 
		 *  one, an empty string is returned.
		 * \return Extracted identifier
		 */
		const std::string& get_identifier() const { return m_identifier_string; }
		const std::string& get_value() const { return m_value_string; }
	private:
		/**
		 * \brief Helper function that reads the next char in the provided source file and advances the accessor caret.
		 */
		void read_char();

		/**
		 * \brief Extracts the next numerical token from the source accessor.
		 * \return Keyword/identifier token, depending on the format and keyword availability
		 */
		token get_identifier_token();

		/**
		 * \brief Extracts the next numerical token from the source accessor.
		 * \return Best-fitting numerical token
		 */
		token get_number_token();
	private:
		std::string m_identifier_string; // current identifier
		std::string m_value_string;      // current value
		std::string m_operator_string;   // current operator string

		detail::string_accessor m_accessor;
		char m_last_character = ' ';

		// tokens that are longer than one character 
		const std::unordered_map<std::string, token> m_keyword_tokens = {
			{ "void", token::definition  },

			// signed integers
			{ "i8"  , token::keyword_i8  },
			{ "i16" , token::keyword_i16 },
			{ "i32" , token::keyword_i32 },
			{ "i64" , token::keyword_i64 },

			// unsigned integers
			{ "u8"  , token::keyword_u8  },
			{ "u16" , token::keyword_u16 },
			{ "u32" , token::keyword_u32 },
			{ "u64" , token::keyword_u64 },

			// floating point
			{ "f32" , token::keyword_f32 },
			{ "f64" , token::keyword_f64 }
		};

		// tokens that consist of special chars (non-alphabetical and non-digit chars), note that the "//"
		// combination is not included here because it is being handled as a comment and thus needs different
		// logic.
		const std::unordered_map<std::string, token> m_special_tokens = {
			{ ";" , token::symbol_semicolon                   },
			{ "(" , token::l_parenthesis                      },
			{ ")" , token::r_parenthesis                      },
			{ "{" , token::l_brace                            },
			{ "}" , token::r_brace                            },
														      
			// operators								      
			{ "+" , token::operator_addition                  },
			{ "+=", token::operator_addition_assignment       },
			{ "-" , token::operator_subtraction               },
			{ "-=", token::operator_subtraction_assignment    },
			{ "*" , token::operator_multiplication            },
			{ "*=", token::operator_multiplication_assignment },
			{ "/",  token::operator_division                  },
			{ "/=", token::operator_division_assignment       },
			{ "=" , token::operator_assignment                },
			{ "==", token::operator_equals                    },
			{ ">" , token::operator_greater_than              },
			{ ">=", token::operator_greater_than_equal_to     },
			{ "<" , token::operator_less_than                 },
			{ "<=", token::operator_less_than_equal_to        },
			{ "!" , token::operator_not                       },
			{ "!=", token::operator_not_equals                },
		};
	};
}