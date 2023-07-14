#include "compilation_unit.h"

namespace sigma {
	compilation_unit::compilation_unit(
		const token_list& token_list
	) {
		m_parser.set_token_list(token_list);
	}

	outcome::result<void> compilation_unit::compile(
		const std::vector<std::shared_ptr<code_generator_context>>& dependencies
	) {
		m_parser.parse();
		// m_parser.get_abstract_syntax_tree()->print_nodes();

		return outcome::success();
	}

	std::shared_ptr<code_generator_context> compilation_unit::get_context() const {
		return m_context;
	}
}
