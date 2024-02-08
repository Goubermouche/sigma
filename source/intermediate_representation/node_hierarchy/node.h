#pragma once
#include "intermediate_representation/node_hierarchy/types.h"
#include "intermediate_representation/node_hierarchy/properties/operations.h"
#include "intermediate_representation/node_hierarchy/properties/memory.h"
#include "intermediate_representation/node_hierarchy/properties/control_flow.h"

#include <utility/containers/allocators/block_allocator.h>
#include <utility/containers/property.h>
#include <utility/containers/slice.h>

namespace sigma::ir {
	struct codegen_context;
	struct user;
	struct symbol;

	using node_properties = utility::property<
		binary_integer_op,
		memory_access,
		projection,
		integer,
		region,
		branch,
		local,
		floating_point_32,
		floating_point_64,
		handle<symbol>,
		function_call,
		array,
		member,
		compare_op
	>;

	/**
	 * \brief Core component of the sea-of-nodes structure of the entire IR system.
	 * Contains information about a given node (mainly its type) and a an optional
	 * (extra) property, which is defined by that type. Nodes are stored in their
	 * respective function in an allocation strategy which stores them and their
	 * respective data right next to each other. \n\n
	 * Nodes also store a list of their inputs (nodes which precede the given node)
	 * in a slice which is also stored right next to them.
	 */
	struct node : node_properties {
		struct type {
			enum underlying : u8 {
				NONE = 0,

				// constants
				INTEGER_CONSTANT,
				F32_CONSTANT,
				F64_CONSTANT,

				// miscellaneous
				// unspecified value, which may be generated by the optimizer when
				// malformed input is folded into an operation
				POISON,
				// extracts a single field from a tuple
				PROJECTION,
				// simple way to embed machine code into 'regular' code
				MACHINE_OPERATION,
				CYCLE_COUNTER,

				PREFETCH,

				// control
				// only one stop and start per function
				ENTRY,
				EXIT,
				// represents a path which has multiple entries, each input is a
				// predecessor
				REGION,
				// since we're using the SSA form we're using phi nodes to select a
				// value from many other values. every phi node chooses its value
				// based on which predecessor was taken to reach it
				PHI,
				// used to implement most control flow, takes a key and matches
				// against some cases, if they match, it'll jump to that successor, if
				// none match it'll take the default successor
				BRANCH,
				// traps in a continuable manner 
				DEBUG_BREAK,
				// not continuable, stops execution
				TRAP,
				// not continuable and unreachable
				UNREACHABLE,
				DEAD,

				// control + memory
				CALL,
				SYSTEM_CALL,
				TAIL_CALL,
				SAFE_POINT_POLL,
				SAFE_POINT_NOP,

				// memory
				MERGE_MEM,
				LOAD,
				STORE,
				MEMCPY,
				MEMSET,
				// these memory accesses represent 'volatile' which means they may
				// produce side effects and thus cannot be eliminated/folded away
				READ,
				WRITE,

				ATOMIC_LOAD,
				ATOMIC_XCHG,
				ATOMIC_ADD,
				ATOMIC_SUB,
				ATOMIC_AND,
				ATOMIC_XOR,
				ATOMIC_OR,
				ATOMIC_CAS,

				LOOKUP,

				// pointers
				// statically allocates stack space
				LOCAL,
				// contains_function a pointer to a symbol object
				SYMBOL,
				// offsets a pointer by a constant value
				MEMBER_ACCESS,
				// base + index * stride
				ARRAY_ACCESS,
				// converts an integer to a pointer
				INTEGER_TO_POINTER,
				// converts a pointer to an integer
				POINTER_TO_INTEGER,

				// conversions
				TRUNCATE,
				FLOAT_EXTEND,
				SIGN_EXTEND,
				ZERO_EXTEND,
				UNSIGNED_INTEGER_TO_FLOAT,
				FLOAT_TO_UNSIGNED_INTEGER,
				INTEGER_TO_FLOAT,
				FLOAT_TO_INTEGER,
				BIT_CAST,

				// select
				SELECT,

				// bit operations
				BIT_SWAP,
				CLZ,
				CTZ,
				POP_COUNT,

				// unary operations
				NOT, // not (?)
				NEG,
				// neg

				// integer arithmetic
				AND,
				OR,
				XOR,
				ADD,
				SUB,
				MUL,

				SHL,
				SHR,
				SAR,
				ROL,
				ROR,
				UDIV,
				SDIV,
				UMOD,
				SMOD,

				// float arithmetic
				FADD,
				FSUB,
				FMUL,
				FDIV,
				FMAX,
				FMIN,

				// comparisons
				CMP_EQ,
				CMP_NE,
				CMP_ULT,
				CMP_ULE,
				CMP_SLT,
				CMP_SLE,
				CMP_FLT,
				CMP_FLE,

				// special operations
				// does full multiplication (64x64=128 and so on) returning the low
				// and high values in separate projections
				MUL_PAIR,

				// variadic
				VARIADIC_START,

				// x86 intrinsics
				X86_INTRINSIC_LDMXCSR,
				X86_INTRINSIC_STMXCSR,
				X86_INTRINSIC_SQRT,
				X86_INTRINSIC_RSQRT
			};

			type();
			type(underlying type);

			operator underlying() const;
		private:
			underlying m_type;
		};

		auto get_type() const -> type;
		void set_type(type type);

		// user stuff
		void add_user(
			handle<node> in, u64 slot, handle<user> recycled, handle<utility::block_allocator> allocator
		);

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
		auto is_unreachable() const -> bool;
		auto is_block_begin() const -> bool;
		auto is_mem_out_op() const -> bool;
		auto is_terminator() const -> bool;
		auto is_block_end() const -> bool;
		auto has_effects() const -> bool;
		auto is_control() const -> bool;
		auto is_pinned() const -> bool;

		handle<user> use;
		utility::slice<handle<node>> inputs; // inputs for the given node
		u64 global_value_index;              // gvi used for optimizations
		data_type dt;                        // generic data type of the node
	private:
		type m_type;
	};

	bool operator==(handle<node> target, node::type::underlying type);
}
