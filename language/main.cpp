#include "source/compiler/compiler.h"

using namespace channel::types;

int main() {
	channel::compiler compiler("test/main.ch", {});

	//channel::lexer lexer("test/main.ch");
	//channel::token token;

	//while ((token = lexer.get_token()) != channel::token::end_of_file) {
	//	std::cout << token_to_string(token);

	//	if (token == channel::token::identifier) {
	//		std::cout << " (" << lexer.get_identifier() << ')';
	//	}
	//	else if (token == channel::token::number_signed ||
	//		token == channel::token::number_unsigned ||
	//		token == channel::token::number_f32 ||
	//		token == channel::token::number_f64) {
	//		std::cout << " (" << lexer.get_value() << ')';
	//	}

	//	std::cout << '\n';
	//}

	return 0;
}