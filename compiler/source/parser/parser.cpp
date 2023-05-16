#include "parser.h"
#include "utility/macros.h"

namespace channel {
	parser::parser()
		: m_abstract_syntax_tree(std::make_shared<abstract_syntax_tree>()) {}

	void parser::set_token_list(const token_list& token_list) {
		m_token_list = token_list;
	}

	const std::shared_ptr<abstract_syntax_tree> parser::get_abstract_syntax_tree() const {
		return m_abstract_syntax_tree;
	}
}