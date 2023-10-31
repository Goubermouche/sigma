#include "function.h"
#include <utility/filesystem/file_types/text_file.h>

namespace ir {
	function::function(const symbol& symbol) :
		m_allocator(NODE_ALLOCATION_BLOCK_SIZE),
		m_symbol(symbol) {
		m_parameters.resize(3);
	}

	void function::print_node_graph(s_ptr<utility::text_file> file) const {
		file << "digraph " << m_symbol.get_name() << "{\n";
		file << "  rankdir=TB\n";

		std::unordered_set<handle<node>> visited;
		m_entry_node->print_as_basic_block(visited, file);

		file << "}\n";
	}

	void function::set_entry_node(handle<node> node) {
		m_entry_node = node;
	}

	handle<node> function::get_entry_node() const {
		return m_entry_node;
	}

	void function::set_exit_node(handle<node> node) {
		m_exit_node = node;
	}

	handle<node> function::get_exit_node() const {
		return m_exit_node;
	}

	void function::set_active_control_node(handle<node> node) {
		m_active_control_node = node;
	}

	handle<node> function::get_active_control_node() const {
		return m_active_control_node;
	}

	u64 function::get_control_node_count() const {
		return m_control_node_count;
	}

	void function::set_control_node_count(u64 count) {
		m_control_node_count = count;
	}

	u64 function::get_node_count() const {
		return m_node_count;
	}

	void function::set_node_count(u64 node_count) {
		m_node_count = node_count;
	}

	symbol& function::get_symbol() {
		return m_symbol;
	}

	const symbol& function::get_symbol() const {
		return m_symbol;
	}

	void function::set_symbol(const symbol& symbol) {
		m_symbol = symbol;
	}

	u64 function::get_parameter_count() const {
		return m_parameter_count;
	}

	void function::set_return_count(u64 return_count) {
		m_return_count = return_count;
	}

	void function::set_parameter(u64 index, handle<node> value) {
		m_parameters[index] = value;
	}

	u64 function::get_return_count() const {
		return m_return_count;
	}

	void function::add_input_late(
		handle<node> n,
		handle<node> input
	) {
		ASSERT(
			n->get_type() == node::type::region || 
			n->get_type() == node::type::phi,
			"invalid node, cannot append input to a region/phi node"
		);

		const u64 old_count = n->m_inputs.get_size();
		const u64 new_count = n->m_inputs.get_size() + 1;

		// reallocate the necessary space again
		utility::slice<handle<node>> new_inputs(
			m_allocator.allocate(sizeof(handle<node>) * new_count), new_count
		);

		memcpy(new_inputs.get_data(), n->get_inputs().get_data(), old_count * sizeof(handle<node>));
		new_inputs[old_count] = input;

		n->m_inputs = new_inputs;
	}

	std::vector<handle<node>>& function::get_parameters() {
		return m_parameters;
	}

	handle<node> function::get_parameter(u64 index) const {
		return m_parameters[index];
	}
}
