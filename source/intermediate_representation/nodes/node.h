#pragma once
#include <utility/containers/slice.h>
#include <utility/containers/property.h>
#include <utility/containers/allocators/block_allocator.h>

#include "intermediate_representation/nodes/data_type.h"
#include "intermediate_representation/nodes/properties/operations.h"
#include "intermediate_representation/nodes/properties/memory.h"
#include "intermediate_representation/nodes/properties/control_flow.h"

namespace ir {
	class user;

	/**
	 * \brief Core component of the sea-of-nodes structure of the entire IR system.
	 * Contains information about a given node (mainly its type) and a an optional
	 * (extra) property, which is defined by that type. Nodes are stored in their
	 * respective function in an allocation strategy which stores them and their
	 * respective data right next to each other. \n\n
	 * Nodes also store a list of their inputs (nodes which precede the given node)
	 * in a slice which is also stored right next to them.
	 */
	class node : public utility::property<
		region_property,
		branch_property,
		projection_property,
		integer_property,
		fp32_property,
		fp64_property,
		binary_integer_op_property,
		local_property,
		memory_access_property
	> {
	public:
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

			// control + memory
			call,
			system_call,
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

		utility::slice<handle<node>> get_inputs() const;
		handle<node> get_input(u64 index) const;
		u64 get_input_count() const;
		type get_type() const;
		u64 get_global_value_index() const;
		data_type& get_data();
		data_type get_data() const;
		std::string get_name() const;
		std::forward_list<handle<user>>& get_users();
		const std::forward_list<handle<user>>& get_users() const;

		void set_input(u64 input, handle<node> node);
		void set_type(type type);
		void set_global_value_index(u64 debug_id);
		void set_data(data_type data);

		void add_user(handle<node> in, i64 slot, handle<user> recycled);
		void replace_input_node(handle<node> input, i64 slot);
		handle<user> remove_user(i64 slot);
		handle<node> get_parent_region();
		handle<node> get_immediate_dominator();
		handle<node> get_block_begin();
		i32 resolve_dominator_depth();
		i32 get_dominator_depth();

		bool has_effects() const;
		bool is_pinned() const;
		bool is_block_end() const;
		bool is_block_begin() const;
		bool is_mem_out_op() const;
		bool should_rematerialize() const;
	private:
		void print_as_basic_block(
			std::unordered_set<handle<node>>& visited,
			s_ptr<utility::text_file> file
		);

		void print_as_node(
			std::unordered_set<handle<node>>& visited,
			s_ptr<utility::text_file> file
		);
	private:
		friend class function;

		utility::slice<handle<node>> m_inputs;   // inputs for the given node
		std::forward_list<handle<user>> m_users; // users of the current node
		type m_type = none;                      // underlying node type
		data_type m_data;                        // additional data 
		u64 m_global_value_index;                // value number used for optimizations 

		// TODO: get rid of this
		static utility::block_allocator s_user_allocator;
	};

	/**
	 * \brief Aligns nodes \a a and \a b in the dominator tree.
	 * \param a Node A
	 * \param b Node B
	 * \return A node or its parent on the same level as the B node.
	 */
	handle<node> align_nodes_in_dominator(handle<node> a, handle<node> b);

	/**
	 * \brief Finds the least common ancestor of nodes \a and \b.
	 * \param a Node A
	 * \param b Node B
	 * \return Least common ancestor of the two nodes.
	 */
	handle<node> find_least_common_ancestor(handle<node> a, handle<node> b);

	/**
	 * \brief Checks if the node \b belongs to node \a a.
	 * \param a Node A
	 * \param b Node B
	 * \return True if node \a is belongs to the \b node block.
	 */
	bool is_same_block(handle<node> a, handle<node> b);
}
