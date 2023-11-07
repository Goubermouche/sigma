#pragma once
#include "intermediate_representation/code_generation/work_list.h"
#include "intermediate_representation/code_generation/live_interval.h"
#include "intermediate_representation/code_generation/machine_block.h"
#include "intermediate_representation/code_generation/targets/target_base.h"
#include "intermediate_representation/code_generation/phi_value.h"
#include "intermediate_representation/nodes/function.h"

// Core code generation context structure. Code generation operations for every 
// function create a new instance of this structure, which is then passed around
// and modified by several functions related to codegen.

namespace ir::cg {
	struct code_generator_context {
		handle<function> function;

		handle<node> fallthrough;

		handle<instruction> first;
		handle<instruction> head;

		utility::block_allocator instruction_allocator;
		s_ptr<target_base> target;
		work_list work_list;

		std::unordered_map<u64, virtual_value> virtual_values;

		std::unordered_map<handle<node>, machine_block> machine_blocks;
		std::unordered_map<handle<node>, i32> stack_slots;
		std::unordered_map<handle<node>, u32> labels;
		std::unordered_map<handle<node>, i32> uses;

		std::vector<live_interval> intervals;
		std::vector<handle<node>> locals;
		std::vector<phi_value> phi_values;

		u32 return_label = 0;
		u64 block_count = 0;
		u64 stack_usage = 0;
		u64 epilogue = 0;

		virtual_value* lookup_value(handle<node> n);
		i32 allocate_stack(u32 size, u32 alignment);

		i32 can_folded_store(
			handle<node> memory, handle<node> address, handle<node> source
		);

		bool on_last_use(handle<node> n);

		template<typename extra_type>
		handle<instruction> create_instruction(
			u64 operand_count
		) {
			void* inst_allocation = instruction_allocator.allocate(sizeof(instruction));
			const handle inst_ptr = static_cast<instruction*>(inst_allocation);

			// assign data
			inst_ptr->set_operands(utility::slice<i32>(instruction_allocator, operand_count));
			inst_ptr->set_property(instruction_allocator.allocate(sizeof(extra_type)));

			return inst_ptr;
		}

		template<typename extra_type>
		handle<instruction> create_instruction(
			instruction::type type,
			data_type data_type,
			u8 out_count,
			u8 in_count,
			u8 tmp_count
		) {
			const handle<instruction> inst = create_instruction<extra_type>(
				out_count + in_count + tmp_count
			);

			inst->set_type(type);
			inst->set_data_type(target->legalize_data_type(data_type));
			inst->set_in_count(in_count);
			inst->set_out_count(out_count);
			inst->set_tmp_count(tmp_count);

			return inst;
		}
	};
}
