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

		std::cout << '\n';
	}

	return 0;
}