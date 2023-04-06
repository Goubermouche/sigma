#pragma once

#include "../lexer/lexer.h"
#include "../codegen/abstract_syntax_tree/node.h"
#include "../type.h"

namespace channel {
	// todo: fix a bug that's causes an error to be thrown when two semicolons
	//       are used right one after another 
	class parser {
	public: 
		parser(const std::string& source_file);

		std::vector<node*> parse();
	private:
		node* parse_statement(bool is_global);

		void consume_next_token();
		void expect_next_token(token token);

		node* parse_declaration(bool is_global, token type_token);
		node* parse_assignment(bool is_global);
		node* parse_expression(token type_token = token::unknown);
		node* parse_term(token type_token = token::unknown);
		node* parse_factor(token type_token = token::unknown);
		node* parse_number(token type_token = token::unknown);
		node* parse_function_call(const std::string& function_name);
		node* parse_function_definition();
		node* parse_return_statement();

		static bool is_token_return_type(token token);
		bool peek_is_function_definition();
		bool peek_is_function_call();
	private:
		lexer m_lexer;
		token m_current_token = token::end_of_file;
	};
}