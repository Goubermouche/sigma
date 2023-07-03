#include "abstract_syntax_tree.h"

namespace sigma {
	void abstract_syntax_tree::add_node(node* node) {
		m_nodes.push_back(node);
	}

	void abstract_syntax_tree::print_nodes() const {
		console::out << "abstract syntax tree\n";

		for(u64 i = 0;i < m_nodes.size(); i++) {
			m_nodes[i]->print(1, L"", i == m_nodes.size() - 1);
		}
	}

	std::vector<node*>::iterator abstract_syntax_tree::begin() {
		return m_nodes.begin();
	}

	std::vector<node*>::iterator abstract_syntax_tree::end() {
		return m_nodes.end();
	}
}
