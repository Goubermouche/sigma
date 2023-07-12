#pragma once
#include "lexer/lexer.h"
#include "code_generator/abstract_syntax_tree/abstract_syntax_tree.h"

namespace sigma {
	class parser {
	public:
		parser();
		virtual ~parser() = default;

		virtual outcome::result<void> parse() = 0;
		void set_token_list(const token_list& token_list);
		std::shared_ptr<abstract_syntax_tree> get_abstract_syntax_tree() const;
	protected:
		// operation temporary
		token_list m_token_list;
		std::shared_ptr<abstract_syntax_tree> m_abstract_syntax_tree;
	};
}
