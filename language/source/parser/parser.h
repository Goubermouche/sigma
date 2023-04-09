#pragma once
#include "../lexer/lexer.h"
#include "../codegen/abstract_syntax_tree/node.h"
#include "../codegen/type.h"

namespace channel {
	class parser {
	public: 
		parser(const std::string& source_file);
		bool parse(std::vector<node*>& abstract_syntax_tree);
	private:
		void consume_next_token();
		bool expect_next_token(token token);

		// statements
		bool parse_global_statement(node*& out_node);
		bool parse_local_statement(node*& out_node);

		// flow control
		bool parse_return_statement(node*& out_node);

		// functions
		bool parse_function_call(node*& out_node, const std::string& function_name);
		bool parse_function_definition(node*& out_node);

		// variables
		bool parse_declaration(node*& out_node, bool is_global, token type_token);
		bool parse_assignment(node*& out_node, bool is_global);

		// numerical
		bool parse_expression(node*& out_node, token type_token = token::unknown);
		bool parse_term(node*& out_node, token type_token = token::unknown);
		bool parse_factor(node*& out_node, token type_token = token::unknown);
		bool parse_number(node*& out_node, token type_token = token::unknown);

		// utility
		node* create_zero_node(type ty) const;
		static bool is_token_return_type(token token);
		bool peek_is_function_definition() const;
		bool peek_is_function_call() const;
	private:
		lexer m_lexer;
		token m_current_token = token::end_of_file;
	};
}