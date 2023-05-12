#pragma once
#include "lexer/token.h"
#include "utility/containers/string_accessor.h"

namespace channel {
	struct token_data {
		token_data() = default;

		token_data(
			token tok, 
			const std::string& value, 
			const token_position& position
		);

		token get_token() const;
		const std::string& get_value() const;
		const token_position& get_token_position() const;
	private:
		token m_token;
		std::string m_value;
		token_position m_position;
	};

	class lexer {
	public:
		lexer(const std::string& source_file);

		bool tokenize();

		void print_tokens() const;

		const token_data& get_token();

		const token_data& peek_token();

		void synchronize_indices();
	private:
		/**
		 * \brief Helper function that reads the next char in the provided source file and advances the accessor caret.
		 */
		void read_char();

		/**
		 * \brief Extracts the next token from the given source file and returns it.
		 * \return Extracted token
		 */
		bool extract_next_token(token& tok);

		/**
		 * \brief Extracts the next numerical token from the source accessor.
		 * \return Keyword/identifier token, depending on the format and keyword availability
		 */
		bool get_identifier_token(token& tok);

		/**
		 * \brief Extracts the next numerical token from the source accessor.
		 * \return Best-fitting numerical token
		 */
		bool get_numerical_token(token& tok);

		bool get_char_literal_token(token& tok);

		bool get_string_literal_token(token& tok);
	private:
		std::vector<token_data> m_tokens;
		u64 m_token_index = 0;
		u64 m_token_peek_index = 0;

		std::string m_source_file;
		std::string m_value_string; 

		detail::string_accessor m_accessor;
		char m_last_character = ' ';

		u64 m_current_line = 1;
		u64 m_current_character = 1;

		// tokens that are longer than one character 
		const std::unordered_map<std::string, token> m_keyword_tokens = {
			{ "void"  , token::keyword_type_void  },
			{ "bool"  , token::keyword_type_bool  },
			{ "true"  , token::bool_literal_true  },
			{ "false" , token::bool_literal_false },

			// signed integers			
			{ "i8"    , token::keyword_type_i8    },
			{ "i16"   , token::keyword_type_i16   },
			{ "i32"   , token::keyword_type_i32   },
			{ "i64"   , token::keyword_type_i64   },

			// unsigned integers		 
			{ "u8"    , token::keyword_type_u8    },
			{ "u16"   , token::keyword_type_u16   },
			{ "u32"   , token::keyword_type_u32   },
			{ "u64"   , token::keyword_type_u64   },

			// floating point			 
			{ "f32"   , token::keyword_type_f32   },
			{ "f64"   , token::keyword_type_f64   },

			// text
			{ "char"  , token::keyword_type_char  },

			// control flow
			{ "return", token::keyword_return     },
			{ "if"    , token::keyword_if         },
			{ "else"  , token::keyword_else       },
			{ "while" , token::keyword_while      },
			{ "for"   , token::keyword_for        },
			{ "break" , token::keyword_break      },
			
			{ "new"   , token::keyword_new        },
		};

		// tokens that consist of special chars (non-alphabetical and non-digit chars), note that the "//"
		// combination is not included here because it is being handled as a comment and thus needs different
		// logic.

		// note: 2-letter tokens need their first token to exist, otherwise the longer version doesn't get
		//       selected and an identifier is returned.
		const std::unordered_map<std::string, token> m_special_tokens = {
			{ "(" , token::l_parenthesis                      },
			{ ")" , token::r_parenthesis                      },
			{ "{" , token::l_brace                            },
			{ "}" , token::r_brace                            },
			{ "[" , token::l_bracket                          },
			{ "]" , token::r_bracket                          },
			{ "," , token::comma                              },
			{ ";" , token::semicolon                          },
			{ "\'", token::single_quote                       },
			{ "\"", token::double_quote                       },

			// operators								      
			{ "+" , token::operator_addition                  },
			{ "++", token::operator_increment                 },
			{ "+=", token::operator_addition_assignment       },
			{ "-" , token::operator_subtraction               },
			{ "--", token::operator_decrement                 },
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