#include "function.h"
#include "intermediate_representation/codegen/work_list.h"

namespace sigma::ir {
	function::function(const std::string& name, linkage linkage, u8 parent_section)
		: symbol(symbol::symbol_type::FUNCTION, name, nullptr, linkage), parent_section(parent_section), allocator(1024), parameters(3) {}

	auto function::get_symbol_address(handle<ir::symbol> target) -> handle<node> {
		ASSERT(target != nullptr, "invalid symbol");

		const handle<node> address = create_node<handle<ir::symbol>>(node::type::SYMBOL, 1);
		address->get<handle<ir::symbol>>() = target;
		address->dt = PTR_TYPE;
		return address;
	}

	void function::create_branch(handle<node> target) {
		const handle<node> control = active_control_node;
		const handle<node> memory = control->peek_memory();

		// there's no need for a branch if the path isn't diverging.
		terminators.push_back(control);
		active_control_node = nullptr;

		// just add the edge directly.
		ASSERT(control->dt == data_type::base::CONTROL, "invalid edge");
		add_input_late(target, control);
		add_memory_edge(control, memory, target);
	}

	auto function::create_call(handle<external> target, const function_signature& call_signature, const std::vector<handle<node>>& arguments) -> handle<node> {
		return create_call(call_signature, get_symbol_address(&target->symbol), arguments);
	}

	auto function::create_call(handle<function> target, const std::vector<handle<node>>& arguments) -> handle<node> {
		return create_call(target->signature, get_symbol_address(&target->symbol), arguments);
	}

	void function::create_conditional_branch(handle<node> condition, handle<node> if_true, handle<node> if_false) {
		const handle<node> memory = active_control_node->get_parent_region()->get<region>().memory_out;

		// generate control projections
		const handle<node> branch = create_node<ir::branch>(node::type::BRANCH, 2);
		branch->dt = TUPLE_TYPE;
		branch->inputs[0] = active_control_node; // control edge
		branch->inputs[1] = condition;

		for(u8 i = 0; i < 2; ++i) {
			const handle<node> target = i ? if_false : if_true;
			const handle<node> c_proj = create_projection(CONTROL_TYPE, branch, i);

			add_input_late(target, c_proj);
			add_memory_edge(branch, memory, target);
		}

		auto& property = branch->get<ir::branch>();
		property.successors.resize(2);
		property.keys.push_back(0);

		terminators.push_back(branch);
		active_control_node = nullptr;
	}

	auto function::create_region() -> handle<node> {
		const handle<node> region = create_node<ir::region>(node::type::REGION, 0);
		region->dt = CONTROL_TYPE;

		const handle<node> phi = create_node<utility::empty_property>(node::type::PHI, 1);
		phi->dt = MEMORY_TYPE;
		phi->inputs[0] = region;

		auto& property = region->get<ir::region>();
		property.memory_in = property.memory_out = phi;
		return region;
	}

	auto function::get_function_parameter(u64 index) const -> handle<node> {
		ASSERT(index < parameter_count, "parameter out of range");
		return parameters[3 + index];
	}

	void function::create_return(const std::vector<handle<node>>& virtual_values) {
		const handle<node> memory_state =active_control_node->peek_memory();

		// allocate a new return node
		if (exit_node == nullptr) {
			const handle<node> exit_region_node = create_node<region>(node::type::REGION, 0);
			exit_region_node->dt = CONTROL_TYPE;

			exit_node = create_node<region>(node::type::EXIT, virtual_values.size() + 3);
			const handle<node> phi_node = create_node<utility::empty_property>(node::type::PHI, 2);

			exit_node->dt = CONTROL_TYPE;
			exit_node->inputs[0] = exit_region_node;
			exit_node->inputs[1] = phi_node;
			exit_node->inputs[2] = parameters[2];

			phi_node->dt = MEMORY_TYPE;
			phi_node->inputs[0] = exit_region_node;
			phi_node->inputs[1] = memory_state;

			for (u64 i = 0; i < virtual_values.size(); ++i) {
				const handle<node> value_phi_node = create_node<utility::empty_property>(
					node::type::PHI, 2
				);

				value_phi_node->dt = virtual_values[i]->dt;
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
			// NOTE: the IR gen currently checks if the exit node is nullptr, and if it is, it adds an
			//       implicit return, this might not work 100% of the time if this gets updated, for more
			//       info see declare_implicit_return()
			NOT_IMPLEMENTED();
		}

		const handle<node> control = active_control_node;
		active_control_node = nullptr;
		add_input_late(exit_node->inputs[0], control);
	}

	auto function::create_signed_integer(i64 value, u8 bit_width) -> handle<node> {
		const handle<node> integer = create_node<ir::integer>(node::type::INTEGER_CONSTANT, 1);
		integer->dt = data_type(data_type::base::INTEGER, bit_width);

		auto& property = integer->get<ir::integer>();
		property.bit_width = bit_width;
		property.value = value;

		return integer;
	}

	auto function::create_unsigned_integer(u64 value, u8 bit_width) -> handle<node> {
		const handle<node> integer = create_node<ir::integer>(node::type::INTEGER_CONSTANT, 1);
		integer->dt = data_type(data_type::base::INTEGER, bit_width);

		if (bit_width < 64) {
			const u64 mask = ~UINT64_C(0) >> (64 - bit_width);
			value &= mask;
		}

		auto& property = integer->get<ir::integer>();
		property.bit_width = bit_width;
		property.value = value;

		return integer;
	}

	auto function::create_bool(bool value) -> handle<node> {
		const handle<node> boolean = create_node<integer>(node::type::INTEGER_CONSTANT, 1);
		boolean->dt = BOOL_TYPE;

		boolean->get<integer>().value = static_cast<u64>(value);
		return boolean;
	}

	auto function::create_add(handle<node> left, handle<node> right,arithmetic_behaviour behaviour) -> handle<node> {
		return create_binary_arithmetic_operation(node::type::ADD, left, right, behaviour);
	}

	auto function::create_sub(handle<node> left, handle<node> right, arithmetic_behaviour behaviour) -> handle<node> {
		return create_binary_arithmetic_operation(node::type::SUB, left, right, behaviour);
	}

	auto function::create_mul(handle<node> left, handle<node> right, arithmetic_behaviour behaviour) -> handle<node> {
		return create_binary_arithmetic_operation(node::type::MUL, left, right, behaviour);
	}

  auto function::create_sxt(handle<node> src, data_type dt) -> handle<node> {
		return create_unary_operation(node::type::SIGN_EXTEND, dt, src);
  }

	auto function::create_zxt(handle<node> src, data_type dt) -> handle<node> {
		return create_unary_operation(node::type::ZERO_EXTEND, dt, src);
	}

	auto function::create_truncate(handle<node> src, data_type dt) -> handle<node> {
		return create_unary_operation(node::type::TRUNCATE, dt, src);
	}

	auto function::create_cmp_eq(handle<node> a, handle<node> b) -> handle<node> {
		return create_cmp_operation(node::type::CMP_EQ, a, b);
	}

	auto function::create_cmp_ne(handle<node> a, handle<node> b) -> handle<node> {
		return create_cmp_operation(node::type::CMP_NE, a, b);
	}

  auto function::create_cmp_ilt(handle<node> a, handle<node> b, bool is_signed) -> handle<node> {
		return create_cmp_operation(is_signed ? node::type::CMP_SLT : node::type::CMP_ULT, a, b);
  }

	auto function::create_cmp_ile(handle<node> a, handle<node> b, bool is_signed) -> handle<node> {
		return create_cmp_operation(is_signed ? node::type::CMP_SLE : node::type::CMP_ULE, a, b);
	}

	auto function::create_cmp_igt(handle<node> a, handle<node> b, bool is_signed) -> handle<node> {
		return create_cmp_operation(is_signed ? node::type::CMP_SLT : node::type::CMP_ULT, b, a);
	}

	auto function::create_cmp_ige(handle<node> a, handle<node> b, bool is_signed) -> handle<node> {
		return create_cmp_operation(is_signed ? node::type::CMP_SLE : node::type::CMP_ULE, b, a);
	}

  auto function::create_not(handle<node> value) -> handle<node> {
		const handle<node> n = create_node<utility::empty_property>(node::type::NOT, 2);
		n->inputs[1] = value;
		return n;
  }

	auto function::create_and(handle<node> a, handle<node> b) -> handle<node> {
		// bitwise operators can't wrap
		return create_binary_arithmetic_operation(node::type::AND, a, b, arithmetic_behaviour::NONE);
	}

	auto function::create_or(handle<node> a, handle<node> b) -> handle<node> {
		return create_binary_arithmetic_operation(node::type::OR, a, b, arithmetic_behaviour::NONE);
	}

	void function::create_store(handle<node> destination, handle<node> value, u32 alignment, bool is_volatile) {
		const handle<node> store = create_node<memory_access>(is_volatile ? node::type::WRITE : node::type::STORE, 4);

		store->dt = MEMORY_TYPE;
		store->inputs[0] = active_control_node;
		store->inputs[1] = append_memory(store);
		store->inputs[2] = destination;
		store->inputs[3] = value;
		store->get<memory_access>().alignment = alignment;
	}

	auto function::create_load(handle<node> value_to_load, data_type data_type, u32 alignment, bool is_volatile) -> handle<node> {
		ASSERT(value_to_load, "invalid value");

		const handle<node> load = create_node<memory_access>(is_volatile ? node::type::READ : node::type::LOAD, 3);

		load->dt = is_volatile ? TUPLE_TYPE : data_type;
		load->inputs[0] = active_control_node;
		load->inputs[1] = active_control_node->get_parent_region()->get<region>().memory_out;
		load->inputs[2] = value_to_load;
		load->get<memory_access>().alignment = alignment;

		if(is_volatile) {
			append_memory(create_projection(MEMORY_TYPE, load, 0));
			return create_projection(data_type, load, 1);
		}

		return load;
	}

  auto function::create_array_access(handle<node> base, handle<node> index, i64 stride) -> handle<node> {
		const handle<node> array = create_node<ir::array>(node::type::ARRAY_ACCESS, 3);
		array->dt = PTR_TYPE;
		array->inputs[1] = base;
		array->inputs[2] = index;

		array->get<ir::array>().stride = stride;
		return array;
  }

	auto function::create_local(u32 size, u32 alignment) -> handle<node> {
		const handle<node> local = create_node<ir::local>(node::type::LOCAL, 1);

		local->inputs[0] = entry_node;
		local->dt = PTR_TYPE;

		auto& property = local->get<ir::local>();
		property.alignment = alignment;
		property.size = size;

		return local;
	}

	auto function::create_call(const function_signature& function_sig, handle<node> callee_symbol_address, const std::vector<handle<node>>& arguments) -> handle<node> {
		const u64 projection_count = 2 + (function_sig.returns.size() > 1 ? function_sig.returns.size() : 1);
		const handle<node> call = create_node<function_call>(node::type::CALL, 3 + arguments.size());

		call->inputs[0] = active_control_node;
		call->inputs[2] = callee_symbol_address;
		call->dt = TUPLE_TYPE;

		for (u64 i = 0; i < arguments.size(); ++i) {
			call->inputs[3 + i] = arguments[i];
		}

		auto& property = call->get<function_call>();
		property.signature = function_sig;

		const handle<node> control_projection = create_projection(CONTROL_TYPE, call, 0);
		const handle<node> memory_projection = create_projection(MEMORY_TYPE, call, 1);
		call->inputs[1] = append_memory(memory_projection);

		// create data projections
		property.projections.resize(projection_count);

		for (u64 i = 0; i < function_sig.returns.size(); ++i) {
			property.projections[i + 2] = create_projection(function_sig.returns[i], call, i + 2);
		}

		if (function_sig.returns.empty()) {
			property.projections[1] = nullptr;
		}

		property.projections[0] = control_projection;
		property.projections[1] = memory_projection;
		active_control_node = control_projection;

		ASSERT(signature.returns.size() <= 1, "invalid return count");
		return property.projections[2];
	}

	auto function::create_binary_arithmetic_operation(node::type type, handle<node> left, handle<node> right, arithmetic_behaviour behaviour) -> handle<node> {
		ASSERT(left->dt == right->dt, "data types of the two operands do not match");
		const handle<node> op = create_node<binary_integer_op>(type, 3);

		op->get<binary_integer_op>().behaviour = behaviour;
		op->inputs[1] = left;
		op->inputs[2] = right;
		op->dt = left->dt;

		return op;
	}

	auto function::create_unary_operation(node::type type, data_type dt, handle<node> src) -> handle<node> {
		const handle<node> operation = create_node<utility::empty_property>(type, 2);
		operation->dt = dt;
		operation->inputs[1] = src;
		return operation;
	}

	auto function::create_cmp_operation(node::type type, handle<node> a, handle<node> b) -> handle<node> {
		ASSERT(a->dt == b->dt, "data types of the two operands do not match");
		const handle<node> cmp = create_node<compare_op>(type, 3);

		cmp->inputs[1] = a;
		cmp->inputs[2] = b;
		cmp->dt = BOOL_TYPE;

		cmp->get<compare_op>().cmp_dt = a->dt;
		return cmp;
	}

	auto function::create_projection(data_type dt, handle<node> source, u64 index) -> handle<node> {
		ASSERT(source->dt == data_type::base::TUPLE, "projections must be of type tuple");
		const handle<node> projection = create_node<ir::projection>(node::type::PROJECTION, 1);

		projection->get<ir::projection>().index = index;
		projection->inputs[0] = source;
		projection->dt = dt;

		return projection;
	}

	auto function::append_memory(handle<node> memory) const -> handle<node> {
		const handle<node> region = active_control_node->get_parent_region();
		auto& property = region->get<ir::region>();
		const handle<node> old_memory = property.memory_out;

		ASSERT(old_memory != nullptr, "invalid memory detected");

		property.memory_out = memory;
		return old_memory;
	}

	void function::add_input_late(handle<node> n, handle<node> input) {
		ASSERT(n == node::type::REGION || n == node::type::PHI, "invalid node, cannot append input to a region/phi node");

		const u64 old_count = n->inputs.get_size();
		const u64 new_count = n->inputs.get_size() + 1;

		// reallocate the necessary space again
		utility::slice<handle<node>> new_inputs(allocator, new_count);

		// copy the old data over to the new slice
		std::memcpy(new_inputs.get_data(), n->inputs.get_data(), old_count * sizeof(handle<node>));

		// add the late input node
		new_inputs[old_count] = input;
		n->inputs = new_inputs;
	}

	void function::add_memory_edge(handle<node> n, handle<node> mem_state, handle<node> target) {
		SUPPRESS_C4100(n);
		ASSERT(target == node::type::REGION, "invalid target type");

		const auto& property = target->get<region>();
		ASSERT(property.memory_in && property.memory_in == node::type::PHI, "invalid region type");

		add_input_late(property.memory_in, mem_state);
	}
} // namespace sigma::ir
