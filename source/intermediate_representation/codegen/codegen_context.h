#pragma once
#include "intermediate_representation/node_hierarchy/function.h"
#include "intermediate_representation/codegen/live_interval.h"
#include "intermediate_representation/codegen/instruction.h"
#include "intermediate_representation/codegen/instruction_operand.h"
#include "intermediate_representation/codegen/work_list.h"
#include "intermediate_representation/codegen/control_flow_graph.h"
#include "intermediate_representation/target/target.h"

#include <utility/containers/allocator_based_containers/linked_list.h>

namespace sigma::ir {
	struct codegen_context {
		/**
		 * \brief Looks up a virtual value for the given node.
		 * \param value Node to look for
		 * \return Respective virtual value.
		 */
		auto lookup_virtual_value(handle<node> value) -> handle<virtual_value>;

		/**
		 * \brief Allocates \b size bytes of stack space aligned with to \b alignment.
		 * \param size Number of bytes to allocate
		 * \param alignment Alignment to allocate the bytes at
		 * \return Stack usage.
		 */
		auto allocate_stack(u64 size, u64 alignment) -> i32;

		/**
		 * \brief Retrieves the stack slot of a given \b node. If the given node does not have an
		 * allocated stack slot, one will be created.
		 * \param node Target node
		 * \return Stack slot index of the given node.
		 */
		auto get_stack_slot(handle<node> node) -> i32;

		/**
		 * \brief Appends the given \b instruction to the end of the instruction list.
		 * \param instruction Instruction to append
		 */
		void append_instruction(handle<instruction> instruction);

		/**
		 * \brief Marks a register \b reg in a given interval as a hint reg, which will be caught by the
		 * register allocator, which will use the hint, if possible.
		 * \param interval_index Index of the interval to hint the register at
		 * \param reg Register to hint
		 */
		void hint_reg(u64 interval_index, reg reg);

		/**
		 * \brief Allocates a new instruction.
		 * \tparam extra_type Optional property of the instruction
		 * \param operand_count Number of operands the instruction is expected to have
		 * \return Newly allocated instruction.
		 */
		template<typename extra_type = utility::empty_property>
		auto create_instruction(u64 operand_count) const -> handle<instruction> {
			void* inst_allocation = function->allocator.allocate_zero(sizeof(instruction));
			const handle inst_ptr = static_cast<instruction*>(inst_allocation);

			// assign data
			inst_ptr->operands = utility::slice<i32>(function->allocator, operand_count);
			inst_ptr->set_property(function->allocator.allocate_zero(sizeof(extra_type)));

			return inst_ptr;
		}

		/**
		 * \brief Allocates a new instruction operand.
		 * \tparam extra_type Optional property of the operand
		 * \return Newly allocated instruction operand.
		 */
		template<typename extra_type = utility::empty_property>
		auto create_instruction_operand() const -> handle<instruction_operand> {
			void* allocation = function->allocator.allocate_zero(sizeof(instruction_operand));
			const handle ptr = static_cast<instruction_operand*>(allocation);

			ptr->set_property(function->allocator.allocate_zero(sizeof(extra_type)));
			return ptr;
		}

		/**
		 * \brief Allocates a new symbol patch.
		 * \return Newly allocated symbol patch.
		 */
		auto create_symbol_patch() const -> handle<symbol_patch>;

		// compilation targets
		handle<function> function;
		target target;

		control_flow_graph graph;
		work_list& work;

		std::vector<u64> basic_block_order;
		utility::contiguous_container<phi_value> phi_values;

		// live intervals which represent value lifetimes 
		std::vector<live_interval> intervals;
		std::vector<u32> labels; 

		// virtual values which represent physical memory / values which are passed around when the
		// program runs 
		std::unordered_map<u64, virtual_value> virtual_values;
		std::unordered_map<handle<node>, i32> stack_slots;

		// schedule generated by the scheduler
		std::unordered_map<handle<node>, handle<basic_block>> schedule;
		std::unordered_map<handle<node>, machine_block> machine_blocks;

		// NOTE: potentially replace with a linked list?
		handle<instruction> first;
		handle<instruction> head;

		// NOTE: potentially replace with a linked list?
		u64 patch_count;
		handle<symbol_patch> first_patch;
		handle<symbol_patch> last_patch;

		// misc
		u64 caller_usage = 0;
		u64 stack_usage = 0;
		u64 endpoint = 0;
		u64 fallthrough;
		u8 prologue_length = 0;
	};
} // namespace sigma::ir
