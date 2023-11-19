#include "module.h"

// register allocation
#include "intermediate_representation/codegen/memory/allocators/linear_scan_allocator.h"

// transformation passes
#include "intermediate_representation/codegen/optimization/optimization_pass_list.h"
#include "intermediate_representation/codegen/transformation/live_range_analysis.h"
#include "intermediate_representation/codegen/transformation/scheduler.h"
#include "intermediate_representation/codegen/transformation/use_list.h"

namespace ir {
	module::~module() {
		for(const handle<function>& function : m_functions) {
			delete function.get();
		}
	}

	auto module::compile(arch arch, system system) const -> codegen_result {
		// specify individual optimization passes
		const optimization_pass_list optimizations({});

		const auto register_allocator = std::make_shared<linear_scan_allocator>();

		const target target = target::create_target(arch, system);
		codegen_result result;

		// go through all declared functions and run codegen
		for (const handle<function>& function : m_functions) {
			utility::console::out << "compiling: " << function->sym->name << '\n';

			// every function has its own unique work list (thread safe), this list
			// is reused in all passes of the given function so that we don't have to
			// reallocate memory needlessly
			work_list function_work_list;

			// initialize the transformation pass
			transformation_context transformation {
				.function = function,
				.work_list = &function_work_list
			};

			// run our transformations
			generate_use_lists(transformation); // mandatory (move over to an optimization?)
			optimizations.apply(transformation);

			// initialize the code generation pass
			codegen_context codegen {
				.function = function,
				.work_list = &function_work_list,
				.intervals = target.get_register_intervals()
			};

			// generate a control flow graph
			codegen.graph = control_flow_graph::compute_reverse_post_order(codegen);

			schedule_node_hierarchy(codegen);
			target.select_instructions(codegen);
			determine_live_ranges(codegen);
			register_allocator->allocate(codegen);
			auto bytecode = target.emit_bytecode(codegen);

			// emit assembly and bytecode
			result.assembly.append(target.disassemble(bytecode, codegen));
			result.bytecode.append(bytecode);
		}

		return result;
	}

	auto module::create_function(
		const std::string& identifier,
		const std::vector<data_type>& parameters, 
		const std::vector<data_type>& returns
	) -> handle<function> {
		handle<function> func = m_functions.emplace_back(
			new function(identifier, symbol::tag::function)
		);

		// allocate the entry node
		const handle<node> entry_node = func->create_node<region>(node::entry, 0);

		auto& entry_region = entry_node->get<region>();

		entry_node->dt = TUPLE_TYPE;
		func->entry_node = entry_node;

		const handle<node> projection_node = func->create_projection(
			CONTROL_TYPE, entry_node, 0
		);

		// initialize the acceleration structure
		func->parameters[0] = projection_node;
		func->parameters[1] = func->create_projection(MEMORY_TYPE, entry_node, 1);
		func->parameters[2] = func->create_projection(CONTINUATION_TYPE, entry_node, 2);

		func->parameter_count = parameters.size();
		func->return_count = returns.size();
		func->active_control_node = projection_node;

		// mark the input memory as both mem_in and mem_out
		entry_region.memory_in = func->parameters[1];
		entry_region.memory_out = func->parameters[1];

		// create parameter projections
		for(u64 i = 0; i < parameters.size(); ++i) {
			const data_type dt = parameters[i];
			func->parameters.push_back(func->create_projection(dt, func->entry_node, 3 + i));
		}

		// TODO: clean up
		std::vector<data_type> par_data(parameters);
		par_data.insert(par_data.end(), returns.begin(), returns.end());

		func->parameter_data = par_data;
		return func;
	}
}
