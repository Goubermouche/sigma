#pragma once
#include "abstract_syntax_tree/node.h"

#include <utility/containers/allocators/block_allocator.h>
#include <utility/containers/contiguous_container.h>

namespace sigma {
	class abstract_syntax_tree {
	public:
		abstract_syntax_tree();

		void traverse(std::function<void(handle<node>, u16)>&& function) const;

		void add_node(handle<node> node);
		auto allocate_node_list(u64 count) -> utility::slice<handle<node>>;

		auto get_nodes() -> utility::contiguous_container<handle<node>>&;
		auto get_nodes() const -> const utility::contiguous_container<handle<node>>&;
		auto get_allocator() -> utility::block_allocator&;

		handle<node> create_binary_expression(node_type type, handle<node> left, handle<node> right);

		template<typename extra_type>
		auto create_node(node_type type, u64 child_count) -> handle<node> {
			const handle node_ptr = static_cast<node*>(m_allocator.allocate(sizeof(node)));

			node_ptr->set_property(m_allocator.allocate(sizeof(extra_type)));
			node_ptr->children = allocate_node_list(child_count);
			node_ptr->type = type;

			return node_ptr;
		}
	private:
		// handles pointing to the main nodes (function and global declarations)
		utility::contiguous_container<handle<node>> m_nodes;

		// the actual node data is stored in a block allocator
		utility::block_allocator m_allocator;
	};
} // namespace sigma
