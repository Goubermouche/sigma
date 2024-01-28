#include "tree.h"
#include "utility/containers/stack.h"

namespace sigma::ast {
	tree::tree() : m_allocator(1024) {}

	void tree::add_node(handle<node> node) {
		m_nodes.push_back(node);
	}

	void tree::traverse(std::function<void(handle<node>, u16)>&& function) const {
		utility::stack<std::pair<handle<node>, u16>> node_stack;

		for(const handle<node>& root_node : m_nodes) {
			node_stack.push_back({ root_node, 0 });

			while (!node_stack.empty()) {
				auto [current, depth] = node_stack.pop_back();

				if(!current) {
					continue;
				}

				function(current, depth);

				for (auto it = current->children.rbegin(); it != current->children.rend(); ++it) {
					node_stack.push_back({ *it, depth + 1 });
				}
			}
		}
	}

	auto tree::allocate_node_list(u16 count) -> utility::slice<handle<node>, u16> {
		return { m_allocator, count };
	}

	auto tree::get_nodes() -> utility::contiguous_container<handle<node>>& {
		return m_nodes;
	}

	auto tree::get_nodes() const -> const utility::contiguous_container<handle<node>>& {
		return m_nodes;
	}

	auto tree::get_allocator() -> utility::block_allocator& {
		return m_allocator;
	}

	auto tree::create_binary_expression(node_type type, handle<node> left, handle<node> right) -> handle<node> {
		const handle<node> node = create_node<utility::empty_property>(type, 2, left->location);
		node->children[0] = left;
		node->children[1] = right;

		return node;
	}
} // namespace sigma::ast
