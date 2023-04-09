#pragma once

#include "../lexer/lexer.h"
#include "../codegen/abstract_syntax_tree/node.h"
#include "../codegen/type.h"

namespace channel {
	// todo: fix a bug that's causes an error to be thrown when two semicolons
	//       are used right one after another 
	class parser {
	public: 
		parser(const std::string& source_file);

		bool parse(std::vector<node*>& abstract_syntax_tree);
	private:
		bool parse_statement(bool is_global, node*& out_node);
		void consume_next_token();
		bool expect_next_token(token token);

		bool parse_declaration(bool is_global, token type_token, node*& out_node);
		bool parse_assignment(bool is_global, node*& out_node);
		bool parse_expression(node*& out_node, token type_token = token::unknown);
		bool parse_term(node*& out_node, token type_token = token::unknown);
		bool parse_factor(node*& out_node, token type_token = token::unknown);
		bool parse_number(node*& out_node, token type_token = token::unknown);
		bool parse_function_call(const std::string& function_name, node*& out_node);
		bool parse_function_definition(node*& out_node);
		bool parse_return_statement(node*& out_node);

		node* create_zero_node(type ty) const;
		static bool is_token_return_type(token token);
		bool peek_is_function_definition() const;
		bool peek_is_function_call() const;
	private:
		lexer m_lexer;
		token m_current_token = token::end_of_file;
	};
}