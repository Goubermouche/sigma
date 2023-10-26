#include "abstract_syntax_tree.h"

namespace sigma {
	void abstract_syntax_tree::add_node(node_ptr node) {
		m_nodes.push_back(node);
	}

	u64 abstract_syntax_tree::size() const {
		return m_nodes.size();
	}

	std::vector<node_ptr>::iterator abstract_syntax_tree::begin() {
		return m_nodes.begin();
	}

	std::vector<node_ptr>::iterator abstract_syntax_tree::end() {
		return m_nodes.end();
	}

	void abstract_syntax_tree::move_insert(
		std::vector<node_ptr>::iterator where,
		s_ptr<abstract_syntax_tree> other
	) {
		std::move(
			std::make_move_iterator(other->m_nodes.begin()),
			std::make_move_iterator(other->m_nodes.end()),
			std::inserter(m_nodes, where)
		);

		other->m_nodes.clear();
	}

	utility::console& operator<<(
		utility::console& console,
		const abstract_syntax_tree& t
	) {
		utility::console::out << "abstract syntax tree\n";

		for (u64 i = 0; i < t.m_nodes.size(); i++) {
			t.m_nodes[i]->print(1, L"", i == t.m_nodes.size() - 1);
		}

		return console;
	}
}
