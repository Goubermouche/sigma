#pragma once
#include "intermediate_representation/node_hierarchy/user.h"
#include "intermediate_representation/node_hierarchy/symbol.h"
#include "intermediate_representation/node_hierarchy/global.h"

#include <utility/containers/byte_buffer.h>

// NOTE: it might be a good idea to determine block size from the node count
//       of the preceding AST and scale relative to that
#define NODE_ALLOCATION_BLOCK_SIZE 1024

namespace sigma::ir {
	struct function;
	struct symbol_patch;

	struct compiled_function {
		handle<function> parent;

		u64 ordinal = 0;
		u8 prologue_length;

		u64 stack_usage;

		u64 code_position; // relative to the export-specific text section
		utility::byte_buffer bytecode;

		u64 patch_count;
		handle<symbol_patch> first_patch;
		handle<symbol_patch> last_patch;
	};

	struct function {
		function(const std::string& name, linkage linkage, u8 parent_section);

		template<typename extra_type>
		auto create_node(node::type type, u64 input_count) -> handle<node>;

		void add_input_late(handle<node> n, handle<node> input);
		void add_memory_edge(handle<node> n, handle<node> mem_state, handle<node> target);

		auto get_symbol_address(handle<symbol> target) -> handle<node>;

		// node hierarchy
		void create_branch(handle<node> target);
		void create_conditional_branch(handle<node> condition, handle<node> if_true, handle<node> if_false);

		auto create_region() -> handle<node>;
		void create_return(const std::vector<handle<node>>& virtual_values);

		auto create_call(handle<external> target, const function_signature& call_signature, const std::vector<handle<node>>& arguments) -> handle<node>;
		auto create_call(handle<function> target, const std::vector<handle<node>>& arguments) -> handle<node>;

		auto create_signed_integer(i64 value, u8 bit_width) -> handle<node>;
		auto create_unsigned_integer(u64 value, u8 bit_width) -> handle<node>;
		auto create_bool(bool value) -> handle<node>;

		auto create_add(handle<node> left, handle<node> right, arithmetic_behaviour behaviour = arithmetic_behaviour::NONE) -> handle<node>;
		auto create_sub(handle<node> left, handle<node> right, arithmetic_behaviour behaviour = arithmetic_behaviour::NONE) -> handle<node>;
		auto create_mul(handle<node> left, handle<node> right, arithmetic_behaviour behaviour = arithmetic_behaviour::NONE) -> handle<node>;

		// casts
		auto create_sxt(handle<node> src, data_type dt) -> handle<node>;
		auto create_zxt(handle<node> src, data_type dt) -> handle<node>;
		auto create_truncate(handle<node> src, data_type dt) -> handle<node>;

		// comparisons
		auto create_cmp_eq(handle<node> a, handle<node> b) -> handle<node>;
		auto create_cmp_ne(handle<node> a, handle<node> b) -> handle<node>;

		auto create_cmp_ilt(handle<node> a, handle<node> b, bool is_signed) -> handle<node>;
		auto create_cmp_ile(handle<node> a, handle<node> b, bool is_signed) -> handle<node>;
		auto create_cmp_igt(handle<node> a, handle<node> b, bool is_signed) -> handle<node>;
		auto create_cmp_ige(handle<node> a, handle<node> b, bool is_signed) -> handle<node>;

		// bitwise operations
		auto create_not(handle<node> value) -> handle<node>;
		auto create_and(handle<node> a, handle<node> b) -> handle<node>;
		auto create_or(handle<node> a, handle<node> b) -> handle<node>;

		void create_store(handle<node> destination, handle<node> value, u32 alignment, bool is_volatile);
		auto create_load(handle<node> value_to_load, data_type data_type, u32 alignment, bool is_volatile) -> handle<node>;
		auto create_array_access(handle<node> base, handle<node> index, i64 stride) -> handle <node>;
		auto create_member_access(handle<node> base, i64 offset) -> handle<node>;

		auto get_function_parameter(u64 index) const-> handle<node>;
		auto create_local(u32 size, u32 alignment) -> handle<node>;
	private:
		auto create_call(const function_signature& function_sig, handle<node> callee_symbol_address,const std::vector<handle<node>>& arguments) -> handle<node>;

		auto create_binary_arithmetic_operation(node::type type, handle<node> left, handle<node> right, arithmetic_behaviour behaviour) -> handle<node>;

		auto create_unary_operation(node::type type, data_type dt, handle<node> src) -> handle<node>;
		auto create_cmp_operation(node::type type, handle<node> a, handle<node> b) -> handle<node>;

		auto create_projection(data_type dt, handle<node> source, u64 index) -> handle<node>;
		auto append_memory(handle<node> memory) const -> handle<node>;
	public:
		symbol symbol;

		compiled_function output;
		function_signature signature;

		u8 parent_section = 0; // index of the parent module section

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

		friend class module;
	};

	template<typename extra_type>
	auto function::create_node(node::type type, u64 input_count) -> handle<node> {
		const handle node_ptr = allocator.allocate_zero<node>();

		// initialize the base sea of nodes layout
		node_ptr->set_type(type);
		node_ptr->set_property(allocator.allocate_zero(sizeof(extra_type)));

		node_ptr->inputs = utility::memory_view<handle<node>>(allocator, input_count);
		node_ptr->global_value_index = node_count++;

		return node_ptr;
	}
} // namespace sigma::ir
