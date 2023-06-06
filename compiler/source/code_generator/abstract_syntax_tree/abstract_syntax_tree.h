#pragma once
#include "code_generator/abstract_syntax_tree/node.h"

namespace channel {
	class abstract_syntax_tree {
	public:
		abstract_syntax_tree() = default;

		void add_node(
			node* node
		);

		void print_nodes() const;

		std::vector<node*>::iterator begin();
		std::vector<node*>::iterator end();
	private:
		std::vector<node*> m_nodes;
	};
}
