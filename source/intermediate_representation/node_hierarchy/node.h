#pragma once
#include "intermediate_representation/node_hierarchy/data_type.h"
#include "intermediate_representation/node_hierarchy/properties/operations.h"
#include "intermediate_representation/node_hierarchy/properties/memory.h"
#include "intermediate_representation/node_hierarchy/properties/control_flow.h"

#include <utility/containers/allocators/block_allocator.h>
#include <utility/containers/property.h>
#include <utility/containers/slice.h>

namespace ir {
	struct codegen_context;
	struct user;
	struct symbol;

	/**
	 * \brief Core component of the sea-of-nodes structure of the entire IR system.
	 * Contains information about a given node (mainly its type) and a an optional
	 * (extra) property, which is defined by that type. Nodes are stored in their
	 * respective function in an allocation strategy which stores them and their
	 * respective data right next to each other. \n\n
	 * Nodes also store a list of their inputs (nodes which precede the given node)
	 * in a slice which is also stored right next to them.
	 */
	struct node : utility::property<
		binary_integer_op, memory_access, projection, integer, region, branch,
		local, floating_point_32, floating_point_64, handle<symbol>, function_call
	> {
		/**
		 * \brief (A, B) -> (C, D) - Every node takes A and B, and produces C and D.
		 * If there's multiple results we need to use projections. The indices are
		 * based on the order seen bellow. Proj0 is C, proj1 is D.
		 * (A, B) & C -> Int - Every node takes A and B along with C in it's extra
		 * data. this is where non-node inputs fit.
		 */
		enum type : u8 {
			none = 0,

			// constants
			integer_constant,
			f32_constant,
			f64_constant,

			// miscellaneous
			// unspecified value, which may be generated by the optimizer when
			// malformed input is folded into an operation
			poison,
			// extracts a single field from a tuple
			projection,
			// simple way to embed machine code into 'regular' code
			machine_operation,
			cycle_counter,

			prefetch,

			// control
			// only one stop and start per function
			entry,
			exit,
			// represents a path which has multiple entries, each input is a
			// predecessor
			region,
			// since we're using the SSA form we're using phi nodes to select a
			// value from many other values. every phi node chooses its value
			// based on which predecessor was taken to reach it
			phi,
			// used to implement most control flow, takes a key and matches
			// against some cases, if they match, it'll jump to that successor, if
			// none match it'll take the default successor
			branch,
			// traps in a continuable manner 
			debug_break,
			// not continuable, stops execution
			trap,
			// not continuable and unreachable
			unreachable,
			dead,

			// control + memory
			call,
			system_call,
			tail_call,
			safe_point_poll,

			// memory
			merge_mem,
			load,
			store,
			memcpy,
			memset,
			// these memory accesses represent 'volatile' which means they may
			// produce side effects and thus cannot be eliminated/folded away
			read,
			write,

			atomic_load,
			atomic_xchg,
			atomic_add,
			atomic_sub,
			atomic_and,
			atomic_xor,
			atomic_or,
			atomic_cas,

			lookup,

			// pointers
			// statically allocates stack space
			local,
			// contains a pointer to a symbol object
			symbol,
			// offsets a pointer by a constant value
			member_access,
			// base + index * stride
			array_access,
			// converts an integer to a pointer
			integer_to_pointer,
			// converts a pointer to an integer
			pointer_to_integer,

			// conversions
			truncate,
			float_extend,
			sign_extend,
			zero_extend,
			uint_to_float,
			float_to_uint,
			int_to_float,
			float_to_int,
			bit_cast,

			// select
			select,

			// bit operations
			bit_swap,
			clz,
			ctz,
			pop_count,

			// unary operations
			negation, // not (?)
			op_not,
			// neg

			// integer arithmetic
			conjunction, // and
			disjunction, // or
			exclusive_disjunction, // xor
			addition,
			subtraction,
			multiplication,

			left_shift,
			right_shift,
			right_arithmetic_shift,
			left_rotate,
			right_rotate,
			unsigned_division,
			signed_division,
			unsigned_modulus,
			signed_modulus,

			// float arithmetic
			float_addition,
			float_subtraction,
			float_multiplication,
			float_division,

			// comparisons
			compare_equal,
			compare_not_equal,
			compare_unsigned_less_than,
			compare_unsigned_less_than_or_equal,
			compare_signed_less_than,
			compare_signed_less_than_or_equal,
			compare_float_less_than,
			compare_float_less_than_or_equal,

			// special operations
			// does full multiplication (64x64=128 and so on) returning the low
			// and high values in separate projections
			multiply_pair,

			// variadic
			variadic_start,

			// x86 intrinsics
			x86_intrinsic_ldmxcsr,
			x86_intrinsic_stmxcsr,
			x86_intrinsic_sqrt,
			x86_intrinsic_rsqrt
		};

		auto get_node_name() const -> std::string;

		// user stuff
		void add_user(handle<node> in, u64 slot, handle<user> recycled, handle<utility::block_allocator> allocator);
		auto has_users(codegen_context& context) -> bool;
		auto remove_user(u64 slot) -> handle<user>;
		void use_node(codegen_context& context);

		// node hierarchy
		auto get_next_control() const -> handle<node>;
		auto peek_memory() -> handle<node>;
		auto get_predecessor(u64 index) -> handle<node>;
		auto get_parent_region() -> handle<node>;
		auto get_block_entry() -> handle<node>;
		auto get_fallthrough() -> handle<node>;
		auto get_next_block() -> handle<node>;

		auto is_critical_edge(handle<node> projection) const -> bool;
		auto is_on_last_use(codegen_context& context) -> bool;
		auto is_control_projection_node() const -> bool;
		auto should_rematerialize() const -> bool;
		auto is_block_begin() const -> bool;
		auto is_mem_out_op() const -> bool;
		auto is_terminator() const -> bool;
		auto is_block_end() const -> bool;
		auto has_effects() const -> bool;
		auto is_control() const -> bool;
		auto is_pinned() const -> bool;

		handle<user> use;
		utility::slice<handle<node>> inputs; // inputs for the given node
		u64 global_value_index;              // value number used for optimizations
		type ty = none;                      // underlying node type
		data_type dt;                        // additional data 
	};
}
