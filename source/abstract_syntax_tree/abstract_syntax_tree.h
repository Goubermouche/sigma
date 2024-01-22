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
		auto allocate_node_list(u16 count) -> utility::slice<handle<node>, u16>;

		auto get_nodes() -> utility::contiguous_container<handle<node>>&;
		auto get_nodes() const -> const utility::contiguous_container<handle<node>>&;
		auto get_allocator() -> utility::block_allocator&;

		auto create_binary_expression(node_type type, handle<node> left, handle<node> right) -> handle<node>;

		template<typename extra_type>
		auto create_node(node_type type, u64 child_count, handle<token_location> location) -> handle<node> {
			const handle node_ptr = m_allocator.emplace<node>();

			node_ptr->set_property(m_allocator.allocate_zero(sizeof(extra_type)));
			node_ptr->children = allocate_node_list(static_cast<u16>(child_count));
			node_ptr->location = location;
			node_ptr->type = type;

			return node_ptr;
		}
	private:
		// handles pointing to the main nodes (functions and globals)
		utility::contiguous_container<handle<node>> m_nodes;

		// the actual node data is stored in a block allocator
		utility::block_allocator m_allocator;
	};
} // namespace sigma
