#include "parser.h"

namespace channel {
	parser::parser(const std::string& source_file)
		: m_lexer(source_file) {
		token token;

		while ((token = m_lexer.get_token()) != channel::token::end_of_file) {
			std::cout << token_to_string(token);

			if (token == channel::token::identifier) {
				std::cout << " (" << m_lexer.get_identifier() << ')';
			}
			else if (token == channel::token::number_signed ||
				token == channel::token::number_unsigned ||
				token == channel::token::number_f32 ||
				token == channel::token::number_f64) {
				std::cout << " (" << m_lexer.get_value() << ')';
			}

			std::cout << '\n';
		}
	}
}