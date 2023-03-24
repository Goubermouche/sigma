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
		std::string m_value_string; // current value
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

		// tokens that are exactly 1 char long
		// note that the '/' token is not included here as it is considered a special
		// token since we also need to check for a comment whenever we see it
		const std::unordered_map<char, token> m_single_tokens = {
			{ '(', token::l_parenthesis      },
			{ ')', token::r_parenthesis      },
			{ '{', token::l_brace            },
			{ '}', token::r_brace            },
			{ '+', token::symbol_plus        },
			{ '-', token::symbol_minus       },
			{ '*', token::symbol_asterisk    },
			{ '=', token::symbol_equals      },
			{ '!', token::symbol_exclamation },
			{ ';', token::symbol_semicolon   },
		};
	};
}