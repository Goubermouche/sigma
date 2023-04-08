#pragma once

#include "token.h"
#include "../utility/string_accessor.h"

namespace channel {
	class lexer {
	public:
		/**
		 * \brief Constructs the lexer for the given \source_file. 
		 * \param source_file Source file to analyze
		 */
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

		/**
		 * \brief Returns the last known value from the lexing process. If there isn't
		 * one, an empty string is returned.
		 * \return Extracted value
		 */
		const std::string& get_value() const { return m_value_string; }

		/**
		 * \brief Returns the last known operator from the lexing process. If there isn't
		 * one, an empty string is returned.
		 * \return Extracted operator
		 */
		const std::string& get_operator() const { return m_operator_string; }

		u64 get_current_line_index() const;
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
		u32 m_current_line = 1;

		// tokens that are longer than one character 
		const std::unordered_map<std::string, token> m_keyword_tokens = {
			{ "void", token::keyword_type_void  },

			// signed integers			
			{ "i8"  , token::keyword_type_i8  },
			{ "i16" , token::keyword_type_i16 },
			{ "i32" , token::keyword_type_i32 },
			{ "i64" , token::keyword_type_i64 },

			// unsigned integers		 
			{ "u8"  , token::keyword_type_u8  },
			{ "u16" , token::keyword_type_u16 },
			{ "u32" , token::keyword_type_u32 },
			{ "u64" , token::keyword_type_u64 },

			// floating point			 
			{ "f32" , token::keyword_type_f32 },
			{ "f64" , token::keyword_type_f64 },

			{ "return", token::keyword_return },
		};

		// tokens that consist of special chars (non-alphabetical and non-digit chars), note that the "//"
		// combination is not included here because it is being handled as a comment and thus needs different
		// logic.

		// note: 2-letter tokens need their first token to exist, otherwise the longer version doesn't get
		//       selected and an identifier is returned.
		const std::unordered_map<std::string, token> m_special_tokens = {
			{ ";" , token::semicolon                          },
			{ "(" , token::l_parenthesis                      },
			{ ")" , token::r_parenthesis                      },
			{ "{" , token::l_brace                            },
			{ "}" , token::r_brace                            },
			{ "," , token::comma                              },
														      
			// operators								      
			{ "+" , token::operator_addition                  },
			{ "+=", token::operator_addition_assignment       },
			{ "-" , token::operator_subtraction               },
			{ "-=", token::operator_subtraction_assignment    },
			{ "*" , token::operator_multiplication            },
			{ "*=", token::operator_multiplication_assignment },
			{ "%" , token::operator_modulo                    },
			{ "%=", token::operator_modulo_assignment         },
			{ "/",  token::operator_division                  },
			{ "/=", token::operator_division_assignment       },
			{ "=" , token::operator_assignment                },
			{ "==", token::operator_equals                    },
			{ ">" , token::operator_greater_than              },
			{ ">>", token::operator_bitwise_right_shift       },
			{ ">=", token::operator_greater_than_equal_to     },
			{ "<" , token::operator_less_than                 },
			{ "<<", token::operator_bitwise_left_shift        },
			{ "<=", token::operator_less_than_equal_to        },
			{ "!" , token::operator_not                       },
			{ "!=", token::operator_not_equals                },
			{ "&" , token::operator_bitwise_and               },
			{ "&&", token::operator_logical_conjunction       },
			{ "|" , token::operator_bitwise_or                },
			{ "||", token::operator_logical_disjunction       },
			{ "~" , token::operator_bitwise_not               },
			{ "^" , token::operator_bitwise_xor               },
		};
	};
}