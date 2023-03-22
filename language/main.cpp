#include "source/lexer/lexer.h"

using namespace language::types;

int main() {
	language::lexer lexer("test/script.lang");
	language::token token;

	while ((token = lexer.get_token()) != language::token::end_of_file) {
		std::cout << token_to_string(token) << std::endl;
	}

	return 0;
}