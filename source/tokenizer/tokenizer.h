#pragma once
#include "tokenizer/token_buffer.h"
#include <utility/containers/string_accessor.h>

namespace sigma {
	class tokenizer {
	public:
		tokenizer(const std::string& source);

		[[nodiscard]] static auto tokenize(
			const std::string& source
		) -> std::pair<token_buffer, utility::symbol_table>;

		[[nodiscard]] auto tokenize() -> std::pair<token_buffer, utility::symbol_table>;
	private:
		[[nodiscard]] auto get_next_token() -> token_info;
		[[nodiscard]] auto get_alphabetical_token() -> token_info;
		[[nodiscard]] auto get_numerical_token() -> token_info;
		[[nodiscard]] auto get_string_literal_token() -> token_info;
		[[nodiscard]] auto get_special_token() -> token_info;

		void consume_spaces();
		auto get_escaped_character() -> char;

		auto get_next_char() -> char;
	private:
		token_location m_current_location = { 0, 0 };
		token_location m_token_start_location; // start location of the current token

		std::string m_current_section;
		char m_last_character = ' '; // prime with a space character

		utility::detail::string_accessor m_source;

		utility::symbol_table m_symbols;
		token_buffer m_tokens;

		// keywords
		const std::unordered_map<std::string, token_type> m_keyword_tokens = {
			// flow control
			{ "ret",  token_type::RET                 },
			{ "if",   token_type::IF                  },
			{ "else", token_type::ELSE                },

			// native type keywords
			{ "i8",    token_type::I8                 },
			{ "i16",   token_type::I16                },
			{ "i32",   token_type::I32                },
			{ "i64",   token_type::I64                },
			{ "bool",  token_type::BOOL               },
			{ "true",  token_type::BOOL_LITERAL_TRUE  },
			{ "false", token_type::BOOL_LITERAL_FALSE },
			{ "false", token_type::BOOL_LITERAL_FALSE },
		};

		const std::unordered_map<std::string, token_type> m_special_tokens = {
			{ "(" ,  token_type::LEFT_PARENTHESIS  },
			{ ")" ,  token_type::RIGHT_PARENTHESIS },
			{ "{" ,  token_type::LEFT_BRACE        },
			{ "}" ,  token_type::RIGHT_BRACE       },
			{ "," ,  token_type::COMMA             },
			{ ";" ,  token_type::SEMICOLON         },
			{ "'" ,  token_type::SINGLE_QUOTE      },
			{ "\"" , token_type::DOUBLE_QUOTE      },
			{ "%" ,  token_type::MODULO            },
			{ "/" ,  token_type::SLASH             },
			{ "*" ,  token_type::ASTERISK          },
			{ "+" ,  token_type::PLUS_SIGN         },
			{ "-" ,  token_type::MINUS_SIGN        },
			{ "=" ,  token_type::EQUALS_SIGN       }
		};
	};
} // namespace sigma
