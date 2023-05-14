#pragma once
#include "lexer/lexer.h"
#include "codegen/abstract_syntax_tree/abstract_syntax_tree.h"

namespace channel {
	class parser {
	public:
		parser();

		virtual error_result parse() = 0;
		void set_token_list(const token_list& token_list);
		const abstract_syntax_tree& get_abstract_syntax_tree() const;
	protected:
		// operation temporary
		token_list m_token_list;
		abstract_syntax_tree m_abstract_syntax_tree;
	};
}
