#include "module.h"
#include <utility/containers/ptr.h>

#include "intermediate_representation/code_generation/use_list.h"
#include "intermediate_representation/code_generation/scheduler.h"
#include "intermediate_representation/code_generation/instruction_selection.h"
#include "intermediate_representation/code_generation/live_range_analysis.h"

#include "intermediate_representation/code_generation/targets/x64_target.h"
#include "intermediate_representation/code_generation/allocators/linear_scan_allocator.h"

namespace ir {
	utility::outcome::result<code_buffer> module::compile(
		arch arch,
		system system
	) {
		ASSERT(
			system == system::windows && arch == arch::x64,
			"unsupported ABI detected"
		);

		const s_ptr<cg::allocator_base> allocator = std::make_shared<cg::linear_scan_allocator>();
		const s_ptr<cg::target_base>    target    = std::make_shared<cg::x64_target>();
		code_buffer buffer;

		// go through all declared functions and generate byte code for them
		// TODO: add multithreading, as long as the symbol system allows for it
		for(auto& function : m_functions) {
			cg::code_generator_context context; // each function has its own context
			context.instruction_allocator = utility::block_allocator(1024);
			context.function = &function;
			context.return_label = 0;
			context.stack_usage = 0;
			context.target = target;

			// initialize dominators
			context.work_list = cg::work_list::post_order(function.get_entry_node());
			context.block_count = context.work_list.get_item_count();
			context.work_list.compute_dominators(function.get_entry_node(), context.block_count);

			// initialize data
			generate_use_lists(context);
			schedule_nodes(context);
			target->allocate_base_registers(context);

			// instruction selection
			context.values.reserve(function.get_node_count());
			context.work_list = cg::work_list::post_order(function.get_entry_node());
			select_instructions(context);

			// determine variable live ranges
			determine_live_ranges(context);

			// allocate registers
			allocator->allocate(context);

			// emit bytecode for the target architecture
			target->emit_code(context, buffer);
		}

		return buffer;
	}

	utility::outcome::result<void> module::print_node_graph(
		const filepath& path
	) const {
		OUTCOME_TRY(const auto& file, utility::text_file::create(path));

		for(auto& function : m_functions) {
			function.print_node_graph(file);
		}

		return utility::outcome::success();
	}

	handle<function> module::create_function(
		const std::string& identifier,
		const std::vector<data_type>& returns
	) {
		auto& func = m_functions.emplace_back(
			function(symbol(identifier, symbol::tag::function))
		);

		// allocate the entry node
		const handle<node> entry_node = create_node<region_property>(
			node::entry, 0
		);

		const handle<region_property> entry_region = entry_node->get<region_property>();
		entry_region->dominator_depth = 0;
		entry_region->dominator = entry_node; // entry node dominates itself
		entry_region->tag = "function entry";

		entry_node->set_data(TUPLE_TYPE);
		func.set_entry_node(entry_node);

		const handle<node> projection_node = create_projection(
			CONTROL_TYPE,
			entry_node,
			0
		);

		// initialize the acceleration structure
		func.set_parameter(0, projection_node);
		func.set_parameter(1, create_projection(MEMORY_TYPE, entry_node, 1));
		func.set_parameter(2, create_projection(CONTINUATION_TYPE, entry_node, 2));

		func.set_return_count(static_cast<i32>(returns.size()));
		func.set_active_control_node(projection_node);

		// mark the input memory as both mem_in and mem_out
		entry_region->memory_in = func.get_parameter(1);
		entry_region->memory_out = func.get_parameter(1);

		// TODO: create parameter projections
		return &func;
	}

	void module::create_ret(const std::vector<handle<node>>& values) {
		const handle<node> memory_state = peek_memory(m_functions.back().get_active_control_node());
		handle<node> exit_node = m_functions.back().get_exit_node();

		// allocate a new return node
		if(exit_node == nullptr) {
			const handle<node> exit_region_node = create_node<region_property>(
				node::region, 0
			);

			exit_node = create_node<region_property>(
				node::exit, values.size() + 3
			);

			const handle<node> phi_node = create_node(
				node::phi, 2
			);

			exit_region_node->set_data(CONTROL_TYPE);

			exit_node->set_data(CONTROL_TYPE);
			exit_node->set_input(0, exit_region_node);
			exit_node->set_input(1, phi_node);
			exit_node->set_input(2, m_functions.back().get_parameters()[2]);

			phi_node->set_data(MEMORY_TYPE);
			phi_node->set_input(0, exit_region_node);
			phi_node->set_input(1, memory_state);

			for(u64 i = 0; i < values.size(); ++i) {
				const handle<node> value_phi_node = create_node(
					node::phi, 2
				);

				value_phi_node->set_data(values[i]->get_data());
				value_phi_node->set_input(0, exit_region_node);
				value_phi_node->set_input(1, values[i]);

				// add the phi node to the exit node
				exit_node->set_input(i + 3, value_phi_node);
			}

			m_functions.back().set_exit_node(exit_node);

			const handle<region_property> exit_region = exit_region_node->get<region_property>();
			exit_region->end = exit_node;
			exit_region->tag = "ret";
			exit_region->memory_in = phi_node;
			exit_region->memory_out = phi_node;
		}
		else {
			ASSERT(false, "not implemented");
		}

		const handle<node> region = exit_node->get_input(0);
		const handle<node> branch = create_node<branch_property>(node::branch, 1);
		branch->set_data(TUPLE_TYPE);
		branch->set_input(0, m_functions.back().get_active_control_node()); // control edge

		std::vector<handle<node>>& successors = add_successors(branch, 1);
		successors[0] = region;
		m_functions.back().set_active_control_node(nullptr);

		m_functions.back().add_input_late(
			region, create_projection(CONTROL_TYPE, branch, 0)
		);
	}

	void module::create_goto(handle<node> target) {
		const handle<node> branch_node = create_node<branch_property>(
			node::branch, 1
		);

		const handle<node> projection_node = create_node<projection_property>(
			node::projection, 1
		);

		branch_node->set_input(0, m_functions.back().get_active_control_node());
		branch_node->set_data(TUPLE_TYPE);

		projection_node->set_data(CONTROL_TYPE);
		projection_node->set_input(0, branch_node);

		std::vector<handle<node>>& successors = add_successors(branch_node, 1);
		successors[0] = target;

		m_functions.back().set_active_control_node(nullptr);
		m_functions.back().add_input_late(target, projection_node);
	}

	void module::create_store(
		handle<node> destination, 
		handle<node> value,
		u32 alignment,
		bool is_volatile
	) {
		const handle<node> store_node = create_node<memory_access_property>(
			is_volatile ? node::write : node::store, 4
		);

		store_node->set_data(MEMORY_TYPE);
		store_node->set_input(0, m_functions.back().get_active_control_node());
		store_node->set_input(1, append_memory(store_node));
		store_node->set_input(2, destination);
		store_node->set_input(3, value);
		store_node->get<memory_access_property>()->alignment = alignment;
	}

	handle<node> module::create_signed_integer(i64 value, u8 bit_width) {
		const handle<node> integer_node = create_node<integer_property>(
			node::integer_constant, 1
		);

		integer_node->set_data(data_type(data_type::id::integer, bit_width));

		const handle<integer_property> data = integer_node->get<integer_property>();
		data->value = value;
		data->bit_width = bit_width;

		return integer_node;
	}

	handle<node> module::create_add(
		handle<node> left,
		handle<node> right, 
		arithmetic_behaviour behaviour
	) {
		return create_binary_arithmetic_operation(
			node::addition,
			left,
			right,
			behaviour
		);
	}

	handle<node> module::create_sub(
		handle<node> left, 
		handle<node> right,
		arithmetic_behaviour behaviour
	) {
		return create_binary_arithmetic_operation(
			node::subtraction,
			left,
			right,
			behaviour
		);
	}

	handle<node> module::create_local(u32 size, u32 alignment) {
		const handle<node> local_node = create_node<local_property>(
			node::local, 1
		);

		local_node->set_data(PTR_TYPE);
		local_node->set_input(0, m_functions.back().get_entry_node());

		const handle<local_property> local = local_node->get<local_property>();
		local->size = size;
		local->alignment = alignment;

		return local_node;
	}

	std::vector<handle<node>>& module::add_successors(
		handle<node> terminator, 
		u64 count
	) const {
		const handle<node> basic_block = m_functions.back().get_active_control_node()->get_parent_region();
		const handle<region_property> region = basic_block->get<region_property>();
		region->end = terminator;

		const handle<branch_property> branch = terminator->get<branch_property>();
		branch->successors.resize(count);
		return branch->successors;
	}

	handle<node> module::create_binary_arithmetic_operation(
		node::type type,
		handle<node> left,
		handle<node> right, 
		arithmetic_behaviour behaviour
	) {
		ASSERT(
			left->get_data() == right->get_data(), 
			"data types of the two operands do not match"
		);

		const handle<node> op_node = create_node<binary_integer_op_property>(
			type, 3
		);

		op_node->get<binary_integer_op_property>()->behaviour = behaviour;
		op_node->set_data(left->get_data());
		op_node->set_input(1, left);
		op_node->set_input(2, right);

		return op_node;
	}

	handle<node> module::create_projection(data_type dt, handle<node> src, i32 index) {
		ASSERT(
			src->get_data().get_id() == data_type::id::tuple,
			"projections must be of type tuple"
		);

		const handle<node> projection_node = create_node<projection_property>(
			node::projection, 1
		);

		projection_node->get<projection_property>()->index = index;
		projection_node->set_input(0, src);
		projection_node->set_data(dt);

		return projection_node;
	}

	handle<node> module::peek_memory(handle<node> control) {
		return control->get_parent_region()->get<region_property>()->memory_out;
	}

	handle<node> module::append_memory(handle<node> new_mem) const {
		const handle<node> block = m_functions.back().get_active_control_node()->get_parent_region();
		const handle<region_property> region = block->get<region_property>();
		const handle<node> old_memory = region->memory_out;

		ASSERT(old_memory != nullptr, "invalid memory");

		region->memory_out = new_mem;
		return old_memory;
	}
}