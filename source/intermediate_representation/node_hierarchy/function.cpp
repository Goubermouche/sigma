#include "function.h"
#include "intermediate_representation/codegen/work_list.h"

namespace ir {
	function::function(const std::string& name) :
		allocator(NODE_ALLOCATION_BLOCK_SIZE)
	{
		symbol.name = name;
		symbol.tag = symbol::symbol_tag::FUNCTION;

		parameters.resize(3);
	}

	auto function::get_symbol_address(handle<ir::symbol> sym) -> handle<node> {
		ASSERT(sym != nullptr, "invalid symbol");

		const handle<node> n = create_node<handle<ir::symbol>>(node::SYMBOL, 1);
		n->get<handle<ir::symbol>>() = sym;
		n->data_type = PTR_TYPE;
		return n;
	}

	void function::create_goto(handle<node> target) {
		const handle<node> n = active_control_node;
		const handle<node> mem_state = n->peek_memory();

		// there's no need for a branch if the path isn't diverging.
		terminators.push_back(n);
		active_control_node = nullptr;

		// just add the edge directly.
		ASSERT(n->data_type.ty == data_type::CONTROL, "invalid edge");
		add_input_late(target, n);
		add_memory_edge(n, mem_state, target);
	}

	auto function::create_call(handle<external> target, const function_type& target_type, const std::vector<handle<node>>& arguments) -> std::vector<handle<node>> {
		return create_call(target_type, get_symbol_address(&target->symbol), arguments);
	}

	auto function::create_call(
		handle<function> target_func, const std::vector<handle<node>>& arguments
	) -> std::vector<handle<node>> {
		return create_call(target_func->type, get_symbol_address(&target_func->symbol), arguments);
	}

	auto function::get_parameter(u64 index) -> handle<node> {
		ASSERT(index < parameter_count, "parameter out of range");
		return parameters[3 + index];
	}

	void function::create_ret(const std::vector<handle<node>>& virtual_values) {
		const handle<node> memory_state =active_control_node->peek_memory();

		// allocate a new return node
		if (exit_node == nullptr) {
			const handle<node> exit_region_node = create_node<region>(node::REGION, 0);
			exit_region_node->data_type = CONTROL_TYPE;

			exit_node = create_node<region>(node::EXIT, virtual_values.size() + 3);
			const handle<node> phi_node = create_node<utility::empty_property>(node::PHI, 2);

			exit_node->data_type = CONTROL_TYPE;
			exit_node->inputs[0] = exit_region_node;
			exit_node->inputs[1] = phi_node;
			exit_node->inputs[2] = parameters[2];

			phi_node->data_type = MEMORY_TYPE;
			phi_node->inputs[0] = exit_region_node;
			phi_node->inputs[1] = memory_state;

			for (u64 i = 0; i < virtual_values.size(); ++i) {
				const handle<node> value_phi_node = create_node<utility::empty_property>(
					node::PHI, 2
				);

				value_phi_node->data_type = virtual_values[i]->data_type;
				value_phi_node->inputs[0] = exit_region_node;
				value_phi_node->inputs[1] = virtual_values[i];

				// add the phi node to the exit node
				exit_node->inputs[i + 3] = value_phi_node;
			}

			auto& exit_region = exit_region_node->get<region>();
			exit_region.memory_out = phi_node;
			exit_region.memory_in = phi_node;

			terminators.push_back(exit_node);
		}
		else {
			ASSERT(false, "not implemented");
		}

		const handle<node> n = active_control_node;
		active_control_node = nullptr;
		add_input_late(exit_node->inputs[0], n);
	}

	auto function::create_signed_integer(i64 value, u8 bit_width) -> handle<node> {
		const handle<node> integer_node = create_node<integer>(node::INTEGER_CONSTANT, 1);
		integer_node->data_type = { .ty = data_type::INTEGER, .bit_width = bit_width };

		auto& integer_prop = integer_node->get<integer>();
		integer_prop.bit_width = bit_width;
		integer_prop.value = value;

		return integer_node;
	}

	auto function::create_add(
		handle<node> left, handle<node> right, arithmetic_behaviour behaviour
	) -> handle<node> {
		return create_binary_arithmetic_operation(
			node::ADD, left, right, behaviour
		);
	}

	auto function::create_sub(
		handle<node> left, handle<node> right, arithmetic_behaviour behaviour
	) -> handle<node> {
		return create_binary_arithmetic_operation(
			node::SUB, left, right, behaviour
		);
	}

	void function::create_store(
		handle<node> destination, handle<node> value, u32 alignment, bool is_volatile
	) {
		const handle<node> store_node = create_node<memory_access>(
			is_volatile ? node::WRITE : node::STORE, 4
		);

		store_node->data_type = MEMORY_TYPE;
		store_node->inputs[0] = active_control_node;
		store_node->inputs[1] = append_memory(store_node);
		store_node->inputs[2] = destination;
		store_node->inputs[3] = value;
		store_node->get<memory_access>().alignment = alignment;
	}

	auto function::create_local(u32 size, u32 alignment) -> handle<node> {
		const handle<node> local_node = create_node<local>(node::LOCAL, 1);

		local_node->inputs[0] = entry_node;
		local_node->data_type = PTR_TYPE;

		auto& local_prop = local_node->get<local>();
		local_prop.alignment = alignment;
		local_prop.size = size;

		return local_node;
	}

	auto function::create_call(
		const function_type& callee_type, handle<node> callee_symbol_address, const std::vector<handle<node>>& arguments
	) ->  std::vector<handle<node>> {
		// const handle<node> target = get_symbol_address(target_func->sym);
		const u64 proj_count = 2 + (callee_type.returns.size() > 1 ? callee_type.returns.size() : 1);

		const handle<node> n = create_node<function_call>(node::CALL, 3 + arguments.size());
		n->inputs[0] = active_control_node;
		n->inputs[2] = callee_symbol_address;
		n->data_type = TUPLE_TYPE;

		for (int i = 0; i < arguments.size(); ++i) {
			n->inputs[3 + i] = arguments[i];
		}

		auto& call_prop = n->get<function_call>();
		call_prop.type = callee_type;

		const handle<node> control_proj = create_projection(CONTROL_TYPE, n, 0);
		const handle<node> memory_proj = create_projection(MEMORY_TYPE, n, 1);
		n->inputs[1] = append_memory(memory_proj);

		// create data projections
		call_prop.projections.resize(proj_count);

		for (u64 i = 0; i < callee_type.returns.size(); ++i) {
			call_prop.projections[i + 2] = create_projection(callee_type.returns[i], n, i + 2);
		}

		// we'll slot a NULL so it's easy to tell when it's empty
		if (callee_type.returns.empty()) {
			call_prop.projections[1] = nullptr;
		}

		call_prop.projections[0] = control_proj;
		call_prop.projections[1] = memory_proj;
		active_control_node = control_proj;

		// todo returns
		return { call_prop.projections.begin() + 2, call_prop.projections.end() };
	}

	auto function::create_binary_arithmetic_operation(node::type op_type, handle<node> left, handle<node> right, arithmetic_behaviour behaviour) -> handle<node> {
		ASSERT(
			left->data_type == right->data_type,
			"data types of the two operands do not match"
		);

		const handle<node> op_node = create_node<binary_integer_op>(op_type, 3);

		op_node->get<binary_integer_op>().behaviour = behaviour;
		op_node->inputs[1] = left;
		op_node->inputs[2] = right;
		op_node->data_type = left->data_type;

		return op_node;
	}

	auto function::create_projection(data_type dt, handle<node> source, u64 index) -> handle<node> {
		ASSERT(source->data_type.ty == data_type::TUPLE, "projections must be of type tuple");
		const handle<node> projection_node = create_node<projection>(node::PROJECTION, 1);

		projection_node->get<projection>().index = index;
		projection_node->inputs[0] = source;
		projection_node->data_type = dt;

		return projection_node;
	}

	auto function::append_memory(handle<node> memory) const -> handle<node> {
		const handle<node> block = active_control_node->get_parent_region();
		auto& region_prop = block->get<region>();
		const handle<node> old_memory = region_prop.memory_out;

		ASSERT(old_memory != nullptr, "invalid memory");

		region_prop.memory_out = memory;
		return old_memory;
	}

	void function::add_input_late(handle<node> n, handle<node> input) {
		ASSERT(
			n->ty == node::type::REGION || 
			n->ty == node::type::PHI,
			"invalid node, cannot append input to a region/phi node"
		);

		const u64 old_count = n->inputs.get_size();
		const u64 new_count = n->inputs.get_size() + 1;

		// reallocate the necessary space again
		utility::slice<handle<node>> new_inputs(allocator, new_count);

		// copy the old data over to the new slice
		memcpy(new_inputs.get_data(), n->inputs.get_data(), old_count * sizeof(handle<node>));

		// add the late input node
		new_inputs[old_count] = input;
		n->inputs = new_inputs;
	}

	void function::add_memory_edge(handle<node> n, handle<node> mem_state, handle<node> target) {
		ASSERT(target->ty == node::REGION, "invalid target type");
		const auto& region_prop = target->get<region>();
		ASSERT(region_prop.memory_in && region_prop.memory_in->ty == node::PHI, "invalid region type");
		add_input_late(region_prop.memory_in, mem_state);
	}
}
