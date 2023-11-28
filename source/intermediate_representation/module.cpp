#include "module.h"

// register allocation
#include "intermediate_representation/codegen/memory/allocators/linear_scan_allocator.h"

// transformation passes
#include "codegen/codegen_context.h"
#include "intermediate_representation/codegen/optimization/optimization_pass_list.h"
#include "intermediate_representation/codegen/transformation/live_range_analysis.h"
#include "intermediate_representation/codegen/transformation/scheduler.h"
#include "intermediate_representation/codegen/transformation/use_list.h"

namespace ir {
	module::module(target target) : m_allocator(1024), m_codegen(target) {
		create_section(".text",  module_section::EXEC,                         comdat::NONE);
		create_section(".data",  module_section::WRITE,                        comdat::NONE);
		create_section(".rdata", module_section::NONE,                         comdat::NONE);
		create_section(".tls$",  module_section::WRITE | module_section::TLS,  comdat::NONE);

		if(target.get_system() == system::WINDOWS) {
			m_chkstk_extern = reinterpret_cast<symbol*>(create_external("__chkstk", SO_LOCAL).get());
		}
	}

	void module::compile() const {
		// specify individual optimization passes
		const optimization_pass_list optimizations({});
		const auto register_allocator = std::make_shared<linear_scan_allocator>();
		utility::string assembly;

		// go through all declared functions and run codegen
		for (const handle<function> function : m_functions) {
			utility::console::out << "compiling function: " << function->symbol.name << '\n';

			// every function has its own unique work list (thread safe), this list
			// is reused in all passes of the given function so that we don't have to
			// reallocate memory needlessly
			work_list function_work_list;

			// initialize the transformation pass
			transformation_context transformation {
				.function  = function,
				.work_list = &function_work_list
			};

			// run our transformations
			generate_use_lists(transformation); // mandatory (move over to an optimization?)
			optimizations.apply(transformation);

			// initialize the code generation pass
			codegen_context codegen {
				.function  = function,
				.work_list = &function_work_list,
				.target    = m_codegen.get_target(),
				.intervals = m_codegen.get_register_intervals()
			};

			// generate a control flow graph
			codegen.graph = control_flow_graph::compute_reverse_post_order(codegen);

			schedule_node_hierarchy(codegen);
			m_codegen.select_instructions(codegen);
			determine_live_ranges(codegen);
			register_allocator->allocate(codegen);
			utility::byte_buffer bytecode = m_codegen.emit_bytecode(codegen);

			// debug - emit an asm-like version of the function
			assembly.append(m_codegen.disassemble(bytecode, codegen));

			// finally, emit the compiled function
			function->output = {
				.parent          = function,
				.prologue_length = codegen.prologue_length,
				.stack_usage     = codegen.stack_usage,
				.bytecode        = bytecode,
				.patches         = codegen.patches
			};
		}

		utility::console::out << assembly.get_underlying() << '\n';
	}

	auto module::generate_object_file() -> utility::object_file {
		return m_codegen.emit_object_file(*this);
	}

	auto module::create_external(const std::string& name, linkage linkage) -> handle<external> {
		const auto ex = static_cast<external*>(m_allocator.allocate(sizeof(external)));
		 
		ex->symbol.tag = symbol::EXTERNAL;
		ex->symbol.name = std::string(name);
		ex->symbol.module = this;
		ex->linkage = linkage;

		m_symbols.insert(&ex->symbol);
		return ex;
	}

	auto module::create_global(const std::string& name, linkage linkage) -> handle<global> {
		const auto g = static_cast<global*>(m_allocator.allocate(sizeof(global)));
		m_globals.emplace_back(g);

		g->symbol.tag = symbol::GLOBAL;
		g->symbol.name = std::string(name);
		g->symbol.module = this;
		g->linkage = linkage;

		m_symbols.insert(&g->symbol);
		return g;
	}

	auto module::create_function(const function_type& type, linkage linkage) -> handle<function> {
		auto func_allocation = m_allocator.allocate(sizeof(function));
		const handle func = new (func_allocation) function(type.identifier);
		m_functions.push_back(func);

		func->linkage = linkage;
		func->parent_section = get_text_section();

		m_symbols.insert(&func->symbol);

		// allocate the entry node
		const handle<node> entry_node = func->create_node<region>(node::ENTRY, 0);

		auto& entry_region = entry_node->get<region>();

		entry_node->data_type = TUPLE_TYPE;
		func->entry_node = entry_node;

		const handle<node> projection_node = func->create_projection(
			CONTROL_TYPE, entry_node, 0
		);

		// initialize the acceleration structure
		func->parameters[0] = projection_node;
		func->parameters[1] = func->create_projection(MEMORY_TYPE, entry_node, 1);
		func->parameters[2] = func->create_projection(CONTINUATION_TYPE, entry_node, 2);

		func->parameter_count = type.parameters.size();
		func->return_count = type.returns.size();
		func->active_control_node = projection_node;

		// mark the input memory as both mem_in and mem_out
		entry_region.memory_in = func->parameters[1];
		entry_region.memory_out = func->parameters[1];

		// create parameter projections
		for(u64 i = 0; i < type.parameters.size(); ++i) {
			const data_type dt = type.parameters[i];
			func->parameters.push_back(func->create_projection(dt, func->entry_node, 3 + i));
		}

		func->type = type;
		return func;
	}

	auto module::create_string(handle<function> f, const std::string& value) -> handle<node> {
		const auto dummy = create_global("", PRIVATE);
		dummy->set_storage(get_rdata_section(), static_cast<u32>(value.size()), 1, 1);

		const auto destination = static_cast<char*>(dummy->add_region(0, static_cast<u32>(value.size())));
		std::memcpy(destination, value.c_str(), value.size());
		return f->get_symbol_address(&dummy->symbol);
	}

	auto module::get_target() const -> target {
		return m_codegen.get_target();
	}

	auto module::get_sections() -> std::vector<module_section>& {
		return m_sections;
	}

	auto module::get_sections() const -> const std::vector<module_section>& {
		return m_sections;
	}

	void module::create_section(const std::string& name, module_section::module_section_flags flags, comdat::comdat_type comdat) {
		const module_section section {
			.name = std::string(name),
			.flags = flags,
			.comdat = {.ty = comdat }
		};

		m_sections.push_back(section);
	}

	auto module::generate_externals() -> std::vector<handle<external>> {
		std::vector<handle<external>> externals = {};

		for(const handle<symbol>& symbol : m_symbols) {
			switch(symbol->tag) {
				case symbol::symbol_tag::FUNCTION: {
					const handle func = reinterpret_cast<function*>(symbol.get());
					const handle section = &m_sections[func->parent_section];

					func->output.ordinal = func->symbol.ordinal;
					section->functions.emplace_back(&func->output);
					break;
				}
				case symbol::symbol_tag::GLOBAL: {
					handle g = reinterpret_cast<global*>(symbol.get());
					m_sections[g->parent_section].globals.push_back(g);
					break;
				}
				case symbol::symbol_tag::EXTERNAL: {
					externals.emplace_back(reinterpret_cast<external*>(symbol.get()));
					break;
				}
				default: {
					break;
				}
			}
		}

		// TODO: actually assign ordinals
		for(module_section& section : m_sections) {
			// sort functions
			std::ranges::sort(section.functions, [](const handle<compiled_function>& a, const handle<compiled_function>& b) {
				return a->ordinal < b->ordinal;
			});

			// sort globals
			std::ranges::sort(section.globals, [](const handle<global>& a, const handle<global>& b) {
				return reinterpret_cast<symbol*>(a.get())->ordinal < reinterpret_cast<symbol*>(b.get())->ordinal;
			});

			// place functions first
			u32 offset = 0;
			for(const auto& function : section.functions) {
				function->code_position = offset;
				offset += static_cast<u32>(function->bytecode.get_size());
			}

			// then globals
			for(const auto& global : section.globals) {
				offset += static_cast<u32>(utility::align(offset, global->alignment));
				global->position = offset;
				offset += global->size;
			}

			section.total_size = offset;
		}

		return externals;
	}
}
