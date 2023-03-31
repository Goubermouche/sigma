#pragma once

#include "../lexer/lexer.h"
//#include "../codegen/abstract_syntax_tree/node.h"

namespace channel {
	class parser {
	public: 
		parser(const std::string& source_file);

		// node* parse_keyword_i32();
	private:
		lexer m_lexer;
		token m_current_token = token::end_of_file;
	};
}