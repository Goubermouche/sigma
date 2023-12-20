#pragma once
#include "intermediate_representation/node_hierarchy/function.h"
#include "intermediate_representation/codegen/live_interval.h"
#include "intermediate_representation/codegen/instruction.h"
#include "intermediate_representation/codegen/codegen_temporary.h"
#include "intermediate_representation/codegen/work_list.h"
#include "intermediate_representation/codegen/control_flow_graph.h"
#include "intermediate_representation/target/target.h"

#include <utility/containers/allocator_based_containers/linked_list.h>

namespace sigma::ir {
	struct codegen_context {
		auto lookup_value(handle<node> value) -> virtual_value*;
		auto allocate_stack(u64 size, u64 alignment) -> i32;
		auto get_stack_slot(handle<node> n) -> i32;

		void append_instruction(handle<instruction> inst);
		void hint_reg(u64 interval_index, reg reg);

		template<typename extra_type = utility::empty_property>
		auto create_instruction(u64 operand_count) const -> handle<instruction> {
			void* inst_allocation = function->allocator.allocate(sizeof(instruction));
			const handle inst_ptr = static_cast<instruction*>(inst_allocation);

			// assign data
			inst_ptr->operands = utility::slice<i32>(function->allocator, operand_count);
			inst_ptr->set_property(function->allocator.allocate(sizeof(extra_type)));

			return inst_ptr;
		}

		template<typename extra_type = utility::empty_property>
		auto create_temporary() const -> handle<codegen_temporary> {
			void* value_allocation = function->allocator.allocate(sizeof(codegen_temporary));
			const handle value_ptr = static_cast<codegen_temporary*>(value_allocation);

			value_ptr->set_property(function->allocator.allocate(sizeof(extra_type)));
			return value_ptr;
		}

		auto create_symbol_patch() const -> handle<symbol_patch>;

		handle<function> function;
		target target;

		control_flow_graph graph;
		work_list& work;

		std::vector<u64> basic_block_order;
		utility::contiguous_container<phi_value> phi_values;

		// live intervals which represent value lifetimes 
		std::vector<live_interval> intervals;
		std::vector<u32> labels; 

		// virtual values which represent physical memory / values which are
		// passed around when the program runs 
		std::unordered_map<u64, virtual_value> virtual_values;
		std::unordered_map<handle<node>, i32> stack_slots;

		// schedule generated by the scheduler
		std::unordered_map<handle<node>, handle<basic_block>> schedule;
		std::unordered_map<handle<node>, machine_block> machine_blocks;

		// TODO: replace with a linked list?
		handle<instruction> first;
		handle<instruction> head;

		u64 patch_count;
		handle<symbol_patch> first_patch;
		handle<symbol_patch> last_patch;

		u64 caller_usage = 0;
		u64 stack_usage = 0;
		u64 endpoint = 0;
		u64 fallthrough;
		u8 prologue_length = 0;
	};
}
