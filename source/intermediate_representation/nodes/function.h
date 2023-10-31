#pragma once
#include "intermediate_representation/nodes/user.h"
#include "intermediate_representation/nodes/symbol.h"

// NOTE: it might be a good idea to determine block size from the node count
//       of the preceding AST and scale relative to that
#define NODE_ALLOCATION_BLOCK_SIZE 1024

namespace ir {
	class function {
	public:
		function(const symbol& symbol);

		void set_parameter(u64 index, handle<node> value);
		void set_active_control_node(handle<node> node);
		void set_return_count(u64 return_count);
		void set_control_node_count(u64 count);
		void set_entry_node(handle<node> node);
		void set_exit_node(handle<node> node);
		void set_symbol(const symbol& symbol);
		void set_node_count(u64 node_count);

		handle<node> get_active_control_node() const;
		std::vector<handle<node>>& get_parameters();
		handle<node> get_parameter(u64 index) const;
		handle<node> get_entry_node() const;
		handle<node> get_exit_node() const;
		u64 get_control_node_count() const;
		const symbol& get_symbol() const;
		u64 get_parameter_count() const;
		u64 get_return_count() const;
		u64 get_node_count() const;
		symbol& get_symbol();

		void print_node_graph(s_ptr<utility::text_file> file) const;

		/**
		 * \brief Allocates a new node, including enough space for the \a extra
		 * \a type and enough input node pointers.
		 * \tparam extra_type Extra type that is contained in the given node
		 * \param type Node type of the node
		 * \param input_count Count of inputs to allocate space for
		 * \return Newly allocated node.
		 */
		template<typename extra_type>
		handle<node> allocate_node(node::type type, u64 input_count);

		/**
		 * \brief Appends a new input to the list of inputs of the \a n node.
		 * Due to the input nodes being stored next to their respective node
		 * we have to reallocate the entire buffer of inputs.
		 * \param n Node to add a new input to
		 * \param input Input node
		 */
		void add_input_late(handle<node> n, handle<node> input);
	private:
		handle<node> m_active_control_node;
		handle<node> m_entry_node;
		handle<node> m_exit_node;

		u64 m_control_node_count = 0;
		u64 m_parameter_count = 0;
		u64 m_return_count = 0;
		u64 m_node_count = 0;

		// list of nodes which are considered parameters
		// 0 - m_parameter_count             : actual parameters
		// m_parameter_count - m_return_count: function returns
		std::vector<handle<node>> m_parameters;

		// allocator which is used for allocating child nodes
		utility::block_allocator m_allocator;
		symbol m_symbol; // symbol of the given function
	};

	template<typename extra_type>
	handle<node> function::allocate_node(node::type type, u64 input_count) {
		void* node_allocation = m_allocator.allocate(sizeof(node));
		const handle node_ptr = new (node_allocation) node(type, m_allocator);

		// initialize the base sea of nodes layout 
		node_ptr->set_inputs(utility::slice<handle<node>>(m_allocator, input_count));
		node_ptr->set_property(m_allocator.allocate(sizeof(extra_type)));
		node_ptr->set_global_value_index(m_node_count++);

		return node_ptr;
	}
}
