#include "source/lexer/lexer.h"

using namespace language::types;

int main() {
	language::lexer lexer("test/script.lang");
	language::token token;

	while ((token = lexer.get_token()) != language::token::end_of_file) {
		std::cout << token_to_string(token);

		if (token == language::token::identifier) {
			std::cout << "       [" << lexer.get_identifier() << ']';
		}
		else if(
			token == language::token::number_signed ||
			token == language::token::number_unsigned ||
			token == language::token::number_f32 ||
			token == language::token::number_f64
			) {
			std::cout << "       [" << lexer.get_value() << ']';
		}

		std::cout << '\n';
	}

	return 0;
}