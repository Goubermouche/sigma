#pragma once
#include "code_generator/abstract_syntax_tree/node.h"

namespace sigma {
	class abstract_syntax_tree {
	public:
		abstract_syntax_tree() = default;

		void add_node(
			node_ptr node
		);

		void print_nodes() const;

		std::vector<node_ptr>::iterator begin();
		std::vector<node_ptr>::iterator end();
	private:
		std::vector<node_ptr> m_nodes;
	};
}
