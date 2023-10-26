#pragma once
#include "abstract_syntax_tree/node.h"

namespace sigma {
	class abstract_syntax_tree {
	public:
		abstract_syntax_tree() = default;

		void add_node(
			node_ptr node
		);

		friend utility::console& operator<<(
			utility::console& console,
			const abstract_syntax_tree& t
		);

		u64 size() const;

		std::vector<node_ptr>::iterator begin();
		std::vector<node_ptr>::iterator end();

		void move_insert(
			std::vector<node_ptr>::iterator where,
			s_ptr<abstract_syntax_tree> other
		);
	private:
		std::vector<node_ptr> m_nodes;
	};
}
