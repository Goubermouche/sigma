#pragma once
#include "node.h"

namespace channel {
	class abstract_syntax_tree {
	public:
		abstract_syntax_tree() = default;

		void add_node(translation_unit_node* node);

		void print_nodes() const;

		std::vector<translation_unit_node*>::iterator begin();
		std::vector<translation_unit_node*>::iterator end();
	private:
		std::vector<translation_unit_node*> m_nodes;
	};
}
