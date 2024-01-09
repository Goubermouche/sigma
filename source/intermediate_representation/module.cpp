#include "module.h"

// register allocation
#include "intermediate_representation/codegen/memory/allocators/linear_scan_allocator.h"

// transformation passes
#include "intermediate_representation/codegen/codegen_context.h"
#include "intermediate_representation/codegen/optimization/optimization_pass_list.h"
#include "intermediate_representation/codegen/transformation/live_range_analysis.h"
#include "intermediate_representation/codegen/transformation/scheduler.h"
#include "intermediate_representation/codegen/transformation/use_list.h"

namespace sigma::ir {
	module::module(target target)
		: m_allocator(1024), m_codegen(target) {
		m_output = m_codegen.generate_sections(*this);
	}

	void module::compile() const {
		// specify individual optimization passes
		const optimization_pass_list optimizations({});
		const auto register_allocator = std::make_shared<linear_scan_allocator>();
		utility::string assembly;

		// go through all declared functions and run codegen
		for (const handle<function>& function : m_functions) {
			// every function has its own unique work list (thread safe), this list is reused in all passes
			// of the given function so that we don't have to reallocate memory needlessly
			work_list function_work_list;

			// initialize the transformation pass
			transformation_context transformation {
				.function = function,
				.work = function_work_list
			};

			// run our transformations
			generate_use_lists(transformation); // mandatory (move over to an optimization?)
			optimizations.apply(transformation);

			// initialize the code generation pass
			codegen_context codegen {
				.function = function,
				.target = m_codegen.get_target(),
				.work = function_work_list,
				.intervals = m_codegen.get_register_intervals()
			};

			// generate a control flow graph
			codegen.graph = control_flow_graph::compute_reverse_post_order(codegen);

			// schedule nodes
			schedule_node_hierarchy(codegen);

			// select instructions for the architecture specified by the target
			m_codegen.select_instructions(codegen);

			// allocate registers (determine live ranges, use these ranges to construct live
			// intervals, which are then used by the selected register allocator.
			determine_live_ranges(codegen);
			register_allocator->allocate(codegen);

			// generate a bytecode representation of the given function for the specified target
			const utility::byte_buffer bytecode = m_codegen.emit_bytecode(codegen);

			// DEBUG
			assembly.append(m_codegen.disassemble(bytecode, codegen));

			// finally, emit the compiled function
			function->output = {
				.parent = function,
				.prologue_length = codegen.prologue_length,
				.stack_usage = codegen.stack_usage,
				.bytecode = bytecode,
				.patch_count = codegen.patch_count,
				.first_patch = codegen.first_patch,
				.last_patch = codegen.last_patch
			};
		}

		// DEBUG
		utility::console::print("{}\n", assembly.get_underlying());
	}

	auto module::generate_object_file() -> utility::byte_buffer {
		return m_codegen.emit_object_file(*this);
	}

	auto module::create_external(const std::string& name, linkage linkage) -> handle<external> {
		const auto memory = m_allocator.allocate(sizeof(external));
		const handle external = new (memory) ir::external();

		// construct the external symbol
		external->symbol = symbol(symbol::EXTERNAL, std::string(name), this, linkage);
		
		m_symbols.emplace_back(&external->symbol);
		return external;
	}

	auto module::create_global(const std::string& name, linkage linkage) -> handle<global> {
		const auto memory = m_allocator.allocate(sizeof(global));
		const handle global = new (memory) ir::global();

		// construct the global
		global->symbol = symbol(symbol::GLOBAL, std::string(name), this, linkage);
		global->objects = std::vector<init_object>();

		m_globals.emplace_back(global);
		m_symbols.emplace_back(&global->symbol);
		return global;
	}

	auto module::create_function(const function_signature& signature, linkage linkage) -> handle<function> {
		const auto memory = m_allocator.allocate(sizeof(function));
		const handle function = new (memory) ir::function(signature.identifier);

		function->symbol.link = linkage;
		function->parent_section = get_text_section();

		m_functions.push_back(function);
		m_symbols.emplace_back(&function->symbol);

		// allocate the entry node
		const handle<node> entry_node = function->create_node<region>(node::type::ENTRY, 0);
		auto& entry_region = entry_node->get<region>();
		entry_node->dt = TUPLE_TYPE;

		function->entry_node = entry_node;
		const handle<node> projection_node = function->create_projection(CONTROL_TYPE, entry_node, 0);

		// initialize the acceleration structure
		function->parameters[0] = projection_node;
		function->parameters[1] = function->create_projection(MEMORY_TYPE, entry_node, 1);
		function->parameters[2] = function->create_projection(CONTINUATION_TYPE, entry_node, 2);

		function->parameter_count = signature.parameters.size();
		function->return_count = signature.returns.size();
		function->active_control_node = projection_node;

		// mark the input memory as both mem_in and mem_out
		entry_region.memory_in = function->parameters[1];
		entry_region.memory_out = function->parameters[1];

		// create parameter projections
		for(u64 i = 0; i < signature.parameters.size(); ++i) {
			const data_type type = signature.parameters[i];
			function->parameters.push_back(function->create_projection(type, function->entry_node, 3 + i));
		}

		function->signature = signature;
		return function;
	}

	auto module::create_string(handle<function> function, const std::string& value) -> handle<node> {
		const auto dummy = create_global("", linkage::PRIVATE);
		dummy->set_storage(get_rdata_section(), static_cast<u32>(value.size() + 1), 1, 1);

		const auto destination = static_cast<char*>(dummy->add_region(0, static_cast<u32>(value.size() + 1)));
		std::memcpy(destination, value.c_str(), value.size() + 1);

		return function->get_symbol_address(&dummy->symbol);
	}

	auto module::get_target() const -> target {
		return m_codegen.get_target();
	}

	auto module::get_output() -> module_output& {
		return m_output;
	}

	auto module::get_output() const -> const module_output& {
		return m_output;
	}

	auto module::generate_externals() -> std::vector<handle<external>> {
		std::vector<handle<external>> externals = {};

		for(const handle<symbol>& symbol : m_symbols) {
			switch(symbol->type) {
				case symbol::symbol_type::FUNCTION: {
					const handle function = reinterpret_cast<ir::function*>(symbol.get());
					const handle section = &m_output.sections[function->parent_section];

					function->output.ordinal = function->symbol.ordinal;
					section->functions.emplace_back(&function->output);
					break;
				}
				case symbol::symbol_type::GLOBAL: {
					const handle global = reinterpret_cast<ir::global*>(symbol.get());

					m_output.sections[global->parent_section].globals.push_back(global);
					break;
				}
				case symbol::symbol_type::EXTERNAL: {
					externals.emplace_back(reinterpret_cast<external*>(symbol.get()));
					break;
				}
				default: {
					break;
				}
			}
		}

		// TODO: actually assign ordinals
		for(module_section& section : m_output.sections) {
			// // sort functions
			// std::ranges::sort(section.functions, [](const handle<compiled_function>& a, const handle<compiled_function>& b) {
			// 	return a->ordinal < b->ordinal;
			// });
			// 
			// // sort globals
			// std::ranges::sort(section.globals, [](const handle<global>& a, const handle<global>& b) {
			// 	return reinterpret_cast<symbol*>(a.get())->ordinal < reinterpret_cast<symbol*>(b.get())->ordinal;
			// });

			// place functions first
			u32 offset = 0;
			for(const handle<compiled_function>& function : section.functions) {
				function->code_position = offset;
				offset += static_cast<u32>(function->bytecode.get_size());
			}

			// then globals
			for(const handle<global>& global : section.globals) {
				offset = static_cast<u32>(utility::align(offset, global->alignment));
				global->position = offset;
				offset += global->size;
			}

			section.total_size = offset;
		}

		return externals;
	}
} // namespace sigma::ir
