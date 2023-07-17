#pragma once
#include "lexer/token.h"
#include "utility/diagnostics/error.h"
#include "utility/containers/string_accessor.h"

namespace sigma {
	/**
	 * \brief Set of information related to a specific token, at a specific location.
	 */
	struct token_data {
		token_data() = default;

		token_data(
			token tok,
			const std::string& value,
			const file_position& position
		);

		token get_token() const;
		const std::string& get_value() const;
		const file_position& get_position() const;
	private:
		// token type representation of the given token
		token m_token = token::unknown;
		// token value of the given token (ie. the identifier value of an identifier token)
		std::string m_value;
		// location of the given token
		file_position m_position;
	};

	/**
	 * \brief Represents a list of tokens. Implements various utility functions for token traversal.
	 */
	class token_list {
	public:
		token_list() = default;
		token_list(
			std::vector<token_data> tokens
		);

		/**
		 * \brief Prints all the contained tokens, if there are any.
		 */
		void print_tokens() const;

		/**
		 * \brief Accesses the next token and advances all indices.
		 * \return Last token.
		 */
		const token_data& get_token();

		/**
		 * \brief Accesses the next token and advances the peek index (while leaving the main index same).
		 * \return Last token.
		 */
		const token_data& peek_token();

		const token_data& get_current_token() const;

		outcome::result<void> expect_token(token token);

		/**
		 * \brief Synchronizes the peek and main indices.
		 */
		void synchronize_indices();
	private:
		std::vector<token_data> m_tokens;

		// todo: implement a check for non-initialized tokens
		token_data m_current_token;

		u64 m_main_token_index = 0;
		u64 m_peek_token_index = 0;
	};

	/**
	 * \brief Base lexer class, used for separating strings into a set of tokens with relevant values.
	 */
	class lexer {
	public:
		/**
		 * \brief Traverses the entire file and generates a list of tokens which can be traversed later.
		 * \return Potentially erroneous result.
		 */
		outcome::result<void> tokenize();
		virtual ~lexer() = default;

		/**
		 * \brief Sets the \a source \a path for the next lexing operation.
		 * \param path Filepath to use
		 * \return Potentially erroneous result.
		 */
		outcome::result<void> set_source_filepath(
			const filepath& path
		);

		token_list get_token_list() const;
	private:
		/**
		 * \brief Helper function that reads the next char in the provided source file and advances the accessor caret.
		 */
		void read_char();

		/**
		 * \brief Extracts the next token from the given source file and returns it.
		 * \return Extracted token
		 */
		outcome::result<void> extract_next_token(
			token& tok
		);

		/**
		 * \brief Extracts the next numerical token from the source accessor.
		 * \return Keyword/identifier token, depending on the format and keyword availability
		 */
		outcome::result<void> get_identifier_token(
			token& tok
		);

		/**
		 * \brief Extracts the next numerical token from the source accessor.
		 * \return Best-fitting numerical token
		 */
		outcome::result<void> get_numerical_token(
			token& tok
		);

		outcome::result<void> get_char_literal_token(
			token& tok
		);

		outcome::result<void> get_string_literal_token(
			token& tok
		);
	private:
		std::vector<token_data> m_tokens;
		filepath m_source_path;
		detail::string_accessor m_accessor;
		char m_last_character = ' ';
		std::string m_value_string;
		u64 m_current_line = 1;
		u64 m_current_character = 1;

		// tokens that are longer than one character 
		const std::unordered_map<std::string, token> m_keyword_tokens = {
			{ "struct" , token::keyword_type_structure },
			{ "void"   , token::keyword_type_void      },
			{ "bool"   , token::keyword_type_bool      },
			{ "include", token::keyword_include        },

			{ "true"   , token::bool_literal_true      },
			{ "false"  , token::bool_literal_false     },

			// signed integers			
			{ "i8"     , token::keyword_type_i8        },
			{ "i16"    , token::keyword_type_i16       },
			{ "i32"    , token::keyword_type_i32       },
			{ "i64"    , token::keyword_type_i64       },

			// unsigned integers		 
			{ "u8"     , token::keyword_type_u8        },
			{ "u16"    , token::keyword_type_u16       },
			{ "u32"    , token::keyword_type_u32       },
			{ "u64"    , token::keyword_type_u64       },

			// floating point			 
			{ "f32"    , token::keyword_type_f32       },
			{ "f64"    , token::keyword_type_f64       },

			// text
			{ "char"   , token::keyword_type_char      },

			// control flow
			{ "return" , token::keyword_return         },
			{ "if"     , token::keyword_if             },
			{ "else"   , token::keyword_else           },
			{ "while"  , token::keyword_while          },
			{ "for"    , token::keyword_for            },
			{ "break"  , token::keyword_break          },

			{ "new"    , token::keyword_new            },
		};

		// tokens that consist of special chars (non-alphabetical and non-digit chars), note that the "//"
		// combination is not included here because it is being handled as a comment and thus needs different
		// logic.
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
			{ "#" , token::hash                               },

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
			{ "/" , token::operator_division                  },
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
