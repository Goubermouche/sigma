#include "node.h"
#include <utility/filesystem/file_types/text_file.h>

#include "intermediate_representation/nodes/user.h"

namespace ir {
	node::node(type type, utility::block_allocator& allocator) 
	: m_type(type), m_allocator(allocator) {}

	utility::slice<handle<node>> node::get_inputs() const {
		return m_inputs;
	}

	handle<node> node::get_input(u64 index) const {
		return m_inputs[index];
	}

	u64 node::get_input_count() const {
		return m_inputs.get_size();
	}

	void node::set_inputs(const utility::slice<handle<node>>& inputs) {
		m_inputs = inputs;
	}

	void node::set_input(u64 input, handle<node> node) {
		m_inputs[input] = node;
	}

	void node::replace_input_node(handle<node> input, i64 slot) {
		// recycle the user
		const handle<user> old_use = remove_user(slot);
		m_inputs[slot] = input;

		if(input != nullptr) {
			add_user(input, slot, old_use);
		}
	}

	node::type node::get_type() const {
		return m_type;
	}

	void node::set_type(type type) {
		m_type = type;
	}

	void node::set_global_value_index(u64 debug_id) {
		m_global_value_index = debug_id;
	}

	u64 node::get_global_value_index() const {
		return m_global_value_index;
	}

	data_type& node::get_data() {
		return m_data;
	}

	data_type node::get_data() const {
		return m_data;
	}

	void node::set_data(data_type data) {
		m_data = data;
	}

	handle<node> node::get_parent_region() {
		handle n = this;

		while (n->get_type() != region && n->get_type() != entry) {
			ASSERT(n->get_input(0) != nullptr, "node has no control edge");
			n = n->get_input(0);
		}

		return n;
	}

	handle<node> node::get_immediate_dominator() {
		handle n = this;

		if(m_type == projection) {
			n = n->get_input(0);
		}

		ASSERT(
			n->get_type() == entry || n->get_type() == region,
			"the given node type does not have a region property"
		);

		return n->get<region_property>()->dominator;
	}

	handle<node> node::get_block_begin() {
		handle n = this;

		while(!n->is_block_begin()) {
			n = n->get_input(0);
		}

		return n;
	}

	i32 node::get_dominator_depth() {
		return get_parent_region()->get<region_property>()->dominator_depth;
	}

	i32 node::resolve_dominator_depth() {
		const i32 dominator_depth = get_dominator_depth();

		if(dominator_depth >= 0) {
			return dominator_depth;
		}

		const i32 parent_dominator_depth = get_immediate_dominator()->resolve_dominator_depth() + 1;
		get<region_property>()->dominator_depth = parent_dominator_depth;
		return parent_dominator_depth;
	}

	std::string node::get_name() const {
		switch (m_type) {
			case none: return "INVALID NODE";

			case entry:  return "entry";
			case exit:   return "exit";
			case projection:   return "proj";
			case region: return "region";

			case local: return "local";

			case variadic_start: return "variadic_start";
			case debug_break: return "debug_break";

			case poison: return "poison";
			case integer_constant: return "int";
			case f32_constant: return "float32";
			case f64_constant: return "float64";

			case phi: return "phi";
			case select: return "select";

			case array_access: return "array";
			case member_access: return "member";

			case pointer_to_integer: return "ptr2int";
			case integer_to_pointer: return "int2ptr";

			case memset: return "memset";
			case memcpy: return "memcpy";

			case zero_extend: return "zxt";
			case sign_extend: return "sxt";
			case float_extend: return "fpxt";
			case truncate: return "trunc";
			case bit_cast: return "bitcast";
			case uint_to_float: return "uint2float";
			case int_to_float: return "int2float";
			case float_to_uint: return "float2uint";
			case float_to_int: return "float2int";
			case symbol: return "symbol";

			case compare_not_equal: return "cmp.ne";
			case compare_equal: return "cmp.eq";
			case compare_unsigned_less_than: return "cmp.ult";
			case compare_unsigned_less_than_or_equal: return "cmp.ule";
			case compare_signed_less_than: return "cmp.slt";
			case compare_signed_less_than_or_equal: return "cmp.sle";
			case compare_float_less_than: return "cmp.lt";
			case compare_float_less_than_or_equal: return "cmp.le";

			case negation: return "not";
			case conjunction: return "and";
			case disjunction: return "or";
			case exclusive_disjunction: return "xor";
			case addition: return "add";
			case subtraction: return "sub";
			case multiplication: return "mul";
			case unsigned_division: return "udiv";
			case signed_division: return "sdiv";
			case unsigned_modulus: return "umod";
			case signed_modulus: return "smod";
			case left_shift: return "shl";
			case right_shift: return "shr";
			case left_rotate: return "rol";
			case right_rotate: return "ror";
			case right_arithmetic_shift: return "sar";

			case float_addition: return "fadd";
			case float_subtraction: return "fsub";
			case float_multiplication: return "fmul";
			case float_division: return "fdiv";

			case multiply_pair: return "mulpair";
			case load: return "load";
			case store: return "store";

			case call: return "call";
			case system_call: return "syscall";
			case branch: return "branch";

			default: {
				ASSERT(false, "unknown type");
				return "";
			}
		}
	}

	std::forward_list<handle<user>>& node::get_users() {
		return m_users;
	}
	
	const std::forward_list<handle<user>>& node::get_users() const {
		return m_users;
	}

	bool node::has_effects() const {
		switch (m_type) {
			// memory effects
			case read:
			case write:
			case store:
			case memcpy:
			case memset:
				return true;
			case projection:
				return m_data.get_id() == data_type::id::control;
			// control flow
			case entry:
			case region:
			case branch:
			case exit:
			case unreachable:
			case debug_break:
			case trap:
			case system_call:
			case call:
				return true;
			default:
				return false;
		}
	}

	bool node::is_pinned() const {
		return
			m_type >= entry && m_type <= safe_point_poll || 
			m_type == projection || 
			m_type == local;
	}

	bool node::is_block_end() const {
		return m_type == branch;
	}

	bool node::is_block_begin() const {
		return
			m_type == region ||
			m_type == projection && m_inputs[0]->get_type() == entry;
	}

	bool node::is_mem_out_op() const { 
		return
			m_type == exit || 
			m_type >= store && m_type <= atomic_cas || 
			m_type == phi && m_data.get_id() == data_type::id::memory;
	}

	bool node::should_rematerialize() const {
		return
			m_type == projection && m_inputs[0]->get_type() == entry ||
			m_type == f32_constant ||
			m_type == f64_constant ||
			m_type == integer_constant ||
			m_type == member_access ||
			m_type == local ||
			m_type == symbol;
	}

	handle<user> node::remove_user(i64 slot) {
		const handle<node> old = m_inputs[slot];
		if (old == nullptr) {
			return nullptr;
		}

		if(old->get_users().empty()) {
			return nullptr;
		}

		// delete the old node from our linked list
		auto prev_it = old->get_users().before_begin();
		for (auto it = old->get_users().begin(); it != old->get_users().end(); ++prev_it, ++it) {
			if ((*it)->get_slot() == slot && (*it)->get_node() == this) {
				const auto old_user = *it;
				old->get_users().erase_after(prev_it);
				return old_user;
			}
		}

		return nullptr;
	}

	void node::add_user(handle<node> in, i64 slot, handle<user> recycled) {
		const auto use = recycled ? recycled : static_cast<user*>(m_allocator.allocate(sizeof(user)));
		 
		use->set_node(this);
		use->set_slot(static_cast<i32>(slot));
		 
		in->get_users().push_front(use);
	}
	
	void node::print_as_basic_block(
		std::unordered_set<handle<node>>& visited,
		utility::long_string& string
	) {
		if(!visited.insert(this).second) {
			return;
		}

		// walk control edges (aka predecessors)
		handle<region_property> region = get<region_property>();
		if(region->end->get_type() == branch) {
			handle<branch_property> branch = region->end->get<branch_property>();
			
			for (u64 i = branch->successors.size(); i-- > 0;) {
				branch->successors[i]->print_as_basic_block(visited, string);
			}
		}

		string.append("  subgraph {\n");
		handle<node> current = region->end;

		do {
			visited.insert(current);
			string.append("    r{} [style = \"filled\"; shape=box; ", current);
			
			if(current->get_type() == exit) {
				string.append(
					"fillcolor=lightblue1; label=\"{}: End",
					current->get_global_value_index()
				);
			}
			else {
				string.append(
					"fillcolor=antiquewhite1; label=\"{}: Effect",
					current->get_global_value_index()
				);
			}

			string.append("\"]\n    r{} -> r{}\n", current->get_input(0), current);
			current = current->get_input(0);
		} while (current != this);

		// basic block header
		string.append("    r{} [style=\"filled\"; shape=box; ", handle(this));
		
		if(m_type == entry) {
			string.append(
				"fillcolor=lightblue1; label=\"{}: Start",
				m_global_value_index
			);
		}
		else {
			string.append(
				"fillcolor=antiquewhite1; label=\"{}: Region", 
				m_global_value_index
			);
		}
		
		string.append("\"]\n");

		if(m_type == entry) {
			string.append("    {{ rank=min; r{} }}\n", handle(this));
		}
		else if (region->end->get_type() == exit) {
			string.append("    {{ rank=max; r{} }}\n", region->end);
		}

		string.append("  }\n");

		// write predecessor edges
		for (handle<node> pred : m_inputs) {
			if(pred->get_type() == projection) {
				string.append(
					"  r{} -> r{}\n", pred->get_input(0), handle(this)
				);
			}
			else {
				string.append(
					"  r{} -> r{}\n", pred, handle(this)
				);
			}
		}

		current = region->end;

		// process adjacent nodes
		do {
			for (u64 i = 1; i < current->get_input_count(); ++i) {
				current->get_input(i)->print_as_node(visited, string);
				string.append("    r{} -> r{}\n", current->get_input(i), current);
			}

			current = current->get_input(0);
		} while (current != this);
 	}

	void node::print_as_node(
		std::unordered_set<handle<node>>& visited,
		utility::long_string& string
	) {
		if (!visited.insert(this).second || has_effects()) {
			return;
		}

		string.append("  r{} [style=\"filled\"; ordering=in; shape=box; fillcolor=lightgrey; label=\"", (u64)this);
		string.append("{}: {}\"];\n", m_global_value_index, get_name());

		for(const handle<node> input : m_inputs) {
			if(input) {
				string.append("  r{} -> r{}\n", input, handle(this));
				input->print_as_node(visited, string);
			}
		}
	}

	handle<node> align_nodes_in_dominator(handle<node> a, handle<node> b) {
		const i32 b_dominator = b->get_dominator_depth();

		while (a->get_input_count() > 0) {
			const handle<node> a_parent = a->get_parent_region();

			if (a_parent->get_dominator_depth() >= b_dominator) {
				break;
			}

			a = a_parent->get<region_property>()->dominator;
		}

		return a;
	}

	handle<node> find_least_common_ancestor(handle<node> a, handle<node> b) {
		if (a == nullptr) {
			return b;
		}

		// align both nodes in the dominator tree
		while (a->get_dominator_depth() > b->get_dominator_depth()) {
			a = a->get_immediate_dominator();
		}

		while (b->get_dominator_depth() > a->get_dominator_depth()) {
			b = b->get_immediate_dominator();
		}

		// walk the dominator tree until both nodes have the same parent node
		while (a != b) {
			b = b->get_immediate_dominator();
			a = a->get_immediate_dominator();
		}

		return a;
	}

	bool is_same_block(handle<node> a, handle<node> b) {
		// filter out nodes without a preceding block in the same region
		if (b->get_type() != node::type::entry && b->get_input(0) == nullptr) {
			return false;
		}

		// find the parent block of the b node
		while (b->get_type() != node::type::entry && b->get_type() != node::type::region) {
			b = b->get_input(0);
		}

		// check if the found block parent is the a node
		return b == a;
	}
}
