#include "abstract_syntax_tree.h"
#include "translation_unit_node.h"

namespace channel {
	void abstract_syntax_tree::add_node(translation_unit_node* node) {
		m_nodes.push_back(node);
	}

	void abstract_syntax_tree::print_nodes() const {
		for(const auto& n : m_nodes) {
			n->print(0, L"", true);
		}
	}

	std::vector<translation_unit_node*>::iterator abstract_syntax_tree::begin() {
		return m_nodes.begin();
	}

	std::vector<translation_unit_node*>::iterator abstract_syntax_tree::end() {
		return m_nodes.end();
	}
}
