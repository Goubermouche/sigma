#pragma once
#include "abstract_syntax_tree/node.h"

#include <utility/allocators/block_allocator.h>
#include <utility/memory/memory_buffer.h>

namespace sigma::ast {
	class tree {
	public:
		tree();

		void traverse(std::function<void(handle<node>, u16)>&& function) const;

		void add_node(handle<node> node);
		auto allocate_node_list(u16 count) -> utility::memory_view<handle<node>, u16>;

		auto get_nodes() -> utility::memory_buffer<handle<node>>&;
		auto get_nodes() const -> const utility::memory_buffer<handle<node>>&;
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
		utility::memory_buffer<handle<node>> m_nodes;

		// the actual node data is stored in a block allocator
		utility::block_allocator m_allocator;
	};
} // namespace sigma::ast
