#pragma once
#include "intermediate_representation/code_generation/code_generator_context.h"

namespace ir::cg {
	/**
	 * \brief Transforms the a node tree into a linear sequence of instructions.
	 * \param context Code generation context
	 */
	void select_instructions(code_generator_context& context);

	/**
	 * \brief Transforms nodes within the given block (starting with \a block).
	 * \param context Code generation context
	 * into a linear sequence of instructions
	 * \param block First node of the region
	 * \param end Last node of the region
	 */
	void select_instructions_region(
		code_generator_context& context,
		handle<node> block,
		handle<node> end
	);

	/**
	 * \brief Selects a specific instruction for a given node based on its type.
	 * \param context Code generation context
	 * \param n Node to select the instruction for
	 * \param destination Potential destination id of a virtual register
	 */
	void select_instruction(
		code_generator_context& context,
		handle<node> n,
		u8 destination
	);

	/**
	 * \brief Selects a specific instruction representing a memory access.
	 * \param context Code generation context
	 * \param n Node to select the instruction for
	 * \param destination The destination register
	 * \param store_op The opcode used for accessing the data
	 * \param source The source register
	 * \return Generated instruction. 
	 */
	handle<instruction> select_memory_access_instruction(
		code_generator_context& context,
		handle<node> n,
		i32 destination,
		i32 store_op,
		i32 source
	);

	/**
	 * \brief Selects a specific instruction representing an array memory access.
	 * \param context Code generation context
	 * \param n Node to select the instruction for
	 * \param destination The destination register
	 * \param store_op The opcode used for accessing the data
	 * \param source The source register
	 * \return Generated instruction. 
	 */
	handle<instruction> select_array_access_instruction(
		code_generator_context& context,
		handle<node> n,
		i32 destination,
		i32 store_op,
		i32 source
	);

	/**
	 * \brief Schedules nodes within the given \a block.
	 * \param context Code generation context
	 * \param block Block to schedule
	 * \param n Currently visited node
	 */
	void dfs_schedule(
		code_generator_context& context,
		handle<node> block,
		handle<node> n
	);

	/**
	 * \brief Schedules the given \a phi node withing a given \a block.
	 * \param context Code generation context
	 * \param block Block to schedule the \a phi for
	 * \param phi Phi node to schedule
	 * \param phi_index Index of the input of the given phi node to schedule
	 */
	void dfs_schedule_phi(
		code_generator_context& context,
		handle<node> block,
		handle<node> phi,
		ptr_diff phi_index
	);

	/**
	 * \brief Appends \a inst to the current head in the given \a context.
	 * \param context Code generation context
	 * \param inst Instruction to append
	 */
	void append_instruction(
		code_generator_context& context,
		handle<instruction> inst
	);

	/**
	 * \brief Obtains a virtual register ID for the given node \a n. If no virtual
	 * register is found a new one is allocated. 
	 * \param context Code generation context
	 * \param n Node to obtain the virtual register ID for
	 * \return ID of the virtual register
	 */
	u8 input_reg(
		code_generator_context& context,
		handle<node> n
	);

	/**
	 * \brief Checks if a node represents a 32-bit integer constant, if it does,
	 * returns its value
	 * \param n Node to check
	 * \param out_x Potential 32-bit value representing the immediate value of the node
	 * \return True if the value represents a 32-bit integer constant, false otherwise.
	 */
	bool try_for_imm32(
		handle<node> n,
		i32& out_x
	);

	/**
	 * \brief Decrements the use count of a node's value description.
	 * \param context Code generation context
	 * \param n Handle to the node whose use count is to be decremented.
	 */
	void use(
		code_generator_context& context,
		handle<node> n
	);

	/**
	 * \brief Retrieves or allocates a stack slot for a given node.
	 * \param context Code generation context
	 * \param n Node for which a stack slot is needed
	 * \return Position of the stack node.
	 */
	i32 get_stack_slot(
		code_generator_context& context,
		handle<node> n
	);

	/**
	 * \brief Provides a register hint for the given interval.
	 * \param context Code generation context
	 * \param interval_index Index of the interval for which a register hint is to be provided
	 * \param reg Register ID to be set as the hint
	 */
	void hint_reg(code_generator_context& context, i32 interval_index, u8 reg);

	/**
	 * \brief Allocates a virtual register for a given node \a n and data type.
	 * \param context Code generation context
	 * \param n Node for which a virtual register is to be allocated
	 * \param data_type Data type of the node for which the register is being allocated
	 * \return ID of the allocated virtual registers.
	 */
	u8 allocate_virtual_register(
		code_generator_context& context,
		handle<node> n,
		const data_type& data_type
	);
}

