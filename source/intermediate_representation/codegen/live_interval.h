#pragma once
#include "intermediate_representation/node_hierarchy/node.h"
#include "intermediate_representation/codegen/memory/memory.h"

#include <util/containers/dense_set.h>
#include <util/range.h>

namespace sigma::ir {
	struct virtual_value {
		u64 use_count = std::numeric_limits<u64>::max();
		reg virtual_register = reg::invalid_id;
	};

	class use_position {
	public:
		enum type {
			OUT,
			REG,
			MEM_OR_REG
		};

		u64 position;
		type type;
	};

	struct phi_value {
		handle<node> phi;
		handle<node> target;

		reg destination;
		reg source;
	};

	struct instruction;

	struct basic_block {
		i32 dominator_depth;
		u64 id;

		handle<node> start;
		handle<node> end;
		handle<node> memory_in;

		handle<basic_block> dominator;
		std::unordered_set<handle<node>> items;
	};

	auto find_least_common_ancestor(handle<basic_block> a, handle<basic_block> b) -> handle<basic_block>;

	struct machine_block {
		u64 terminator;
		u64 start;
		u64 end;

		handle<instruction> first;
		handle<node> end_node;

		utility::dense_set gen;
		utility::dense_set kill;
		utility::dense_set live_in;
		utility::dense_set live_out;
	};

	struct symbol_patch {
		handle<symbol_patch> next;
		handle<function> source;
		handle<symbol> target;

		bool internal;
		u64 pos;
	};

	struct live_interval {
		auto get_start() const -> u64;
		auto get_end() const -> u64;

		void add_range(const utility::range<u64>& range);
		auto split_at(codegen_context& context, u64 position) -> handle<live_interval>;

		reg assigned;
		reg hint;

		classified_reg reg;
		handle<node> target;

		u64 active_range = 0;

		i32 data_type; // generic data type (= instruction.data_type)
		i32 split_child = -1;
		i32 spill = -1;

		std::vector<utility::range<u64>> ranges;
		std::vector<use_position> uses;
	};

	ptr_diff interval_intersect(handle<live_interval> a, handle<live_interval> b);
} // namespace sigma::ir
