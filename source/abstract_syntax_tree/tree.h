#pragma once
#include "abstract_syntax_tree/node.h"

#include <util/block_allocator.h>
#include <util/containers/contiguous_buffer.h>

namespace sigma::ast {
	class tree {
	public:
		tree();

		void traverse(std::function<void(handle<node>, u16)>&& function) const;

		void add_node(handle<node> node);
		auto allocate_node_list(u16 count) -> utility::slice<handle<node>, u16>;

		auto get_nodes() -> utility::contiguous_buffer<handle<node>>&;
		auto get_nodes() const -> const utility::contiguous_buffer<handle<node>>&;
		auto get_allocator() -> utility::block_allocator&;

		template<typename extra_type = utility::empty_property>
		auto create_node(node_type type, u64 child_count, handle<token_location> location) -> handle<node> {
			ASSERT(child_count <= std::numeric_limits<u16>::max(), "cannot allocate more than {} children", std::numeric_limits<u16>::max());
			const handle node_ptr = m_allocator.emplace<node>();

			// initialize the node
			node_ptr->set_property(m_allocator.emplace<extra_type>());
			node_ptr->children = allocate_node_list(static_cast<u16>(child_count));
			node_ptr->location = location;
			node_ptr->type = type;

			return node_ptr;
		}
	private:
		// handles pointing to the main nodes (functions and globals)
		utility::contiguous_buffer<handle<node>> m_nodes;

		// the actual node data is stored in a block allocator
		utility::block_allocator m_allocator;
	};
} // namespace sigma::ast
