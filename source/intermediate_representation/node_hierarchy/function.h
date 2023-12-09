#pragma once
#include "intermediate_representation/node_hierarchy/user.h"
#include "intermediate_representation/node_hierarchy/symbol.h"
#include "intermediate_representation/node_hierarchy/global.h"

#include <utility/containers/byte_buffer.h>
#include <utility/containers/allocator_based_containers/linked_list.h>

// NOTE: it might be a good idea to determine block size from the node count
//       of the preceding AST and scale relative to that
#define NODE_ALLOCATION_BLOCK_SIZE 1024

namespace ir {
	struct function;
	struct symbol_patch;

	struct compiled_function {
		handle<function> parent;

		u64 ordinal = 0;
		u8 prologue_length;

		uint64_t stack_usage;

		u64 code_position; // relative to the export-specific text section
		utility::byte_buffer bytecode;

		u64 patch_count;
		handle<symbol_patch> first_patch;
		handle<symbol_patch> last_patch;
	};

	struct function {
		function() = default;
		function(const std::string& name);

		template<typename extra_type>
		auto create_node(node::type type, u64 input_count) -> handle<node>;

		void add_input_late(handle<node> n, handle<node> input);
		void add_memory_edge(handle<node> n, handle<node> mem_state, handle<node> target);

		auto get_symbol_address(handle<symbol> sym) -> handle<node>;

		// node hierarchy
		void create_branch(handle<node> target);
		void create_conditional_branch(handle<node> condition, handle<node> if_true, handle<node> if_false);

		auto create_region() -> handle<node>;

		void create_ret(const std::vector<handle<node>>& virtual_values);

		auto create_call(handle<external> target, const function_type& target_type, const std::vector<handle<node>>& arguments) -> std::vector<handle<node>>;
		auto create_call(handle<function> target_func, const std::vector<handle<node>>& arguments) -> std::vector<handle<node>>;

		auto create_signed_integer(i64 value, u8 bit_width) -> handle<node>;
		auto create_bool(bool value) -> handle<node>;

		auto create_add(handle<node> left, handle<node> right, arithmetic_behaviour behaviour = arithmetic_behaviour::none) -> handle<node>;
		auto create_sub(handle<node> left, handle<node> right, arithmetic_behaviour behaviour = arithmetic_behaviour::none) -> handle<node>;
		auto create_mul(handle<node> left, handle<node> right, arithmetic_behaviour behaviour = arithmetic_behaviour::none) -> handle<node>;

		void create_store(handle<node> destination, handle<node> value, u32 alignment, bool is_volatile);
		auto create_load(handle<node> value_to_load, data_type data_type, u32 alignment, bool is_volatile) -> handle<node>;

		auto get_function_parameter(u64 index) -> handle<node>;
		auto create_local(u32 size, u32 alignment) -> handle<node>;
	private:
		auto create_call(const function_type& callee_type, handle<node> callee_symbol_address, const std::vector<handle<node>>& arguments) -> std::vector<handle<node>>;
		auto create_binary_arithmetic_operation(node::type op_type, handle<node> left, handle<node> right, arithmetic_behaviour behaviour) -> handle<node>;
		auto create_projection(data_type dt, handle<node> source, u64 index) -> handle<node>;
		auto append_memory(handle<node> memory) const -> handle<node>;
	public:
		friend class module;
		symbol symbol; // symbol of the given function

		compiled_function output;
		function_type type;
		linkage linkage;

		u8 parent_section; // index of the parent module section

		// allocator which is used for allocating child nodes
		utility::block_allocator allocator;

		handle<node> active_control_node;
		handle<node> entry_node;
		handle<node> exit_node;
		
		u64 control_node_count = 0;
		u64 parameter_count = 0;
		u64 return_count = 0;
		u64 node_count = 0;

		// list of nodes which are considered parameters
		// 0 - m_parameter_count             : actual parameters
		// m_parameter_count - m_return_count: function returns
		std::vector<handle<node>> parameters;
		std::vector<handle<node>> terminators;
	};

	template<typename extra_type>
	auto function::create_node(node::type type, u64 input_count) -> handle<node> {
		void* node_allocation = allocator.allocate(sizeof(node));
		const handle node_ptr = new (node_allocation) node{ .ty = type };

		// initialize the base sea of nodes layout 
		node_ptr->inputs = utility::slice<handle<node>>(allocator, input_count);
		node_ptr->set_property(allocator.allocate(sizeof(extra_type)));
		node_ptr->global_value_index = node_count++;

		return node_ptr;
	}
}
