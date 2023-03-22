#include "source/lexer/lexer.h"

using namespace language::types;

int main() {
	language::lexer lexer("test/script.lang");

	language::token token = lexer.get_token();

	while(token != language::token::end_of_file) {
		std::cout << static_cast<i32>(token) << '\n';
		token = lexer.get_token();
	}

	return 0;
}