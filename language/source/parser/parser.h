#pragma once

#include "../lexer/lexer.h"
#include "../codegen/abstract_syntax_tree/node.h"

namespace channel {
	class parser {
	public: 
		parser(const std::string& source_file);

		std::vector<node*> parse();
	private:
		node* parse_statement();

		void consume_next_token();
		void expect_next_token(token token);

		node* parse_declaration_or_assignment();
		node* parse_expression();
		node* parse_term();
		node* parse_factor();
		node* parse_number();
		node* parse_function_call(const std::string& function_name);
		node* parse_function_definition();

		static bool is_token_return_type(token token);
	private:
		lexer m_lexer;
		token m_current_token = token::end_of_file;
	};
}