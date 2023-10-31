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

	/**
	 * \brief Creates a register-register-memory (RRM) instruction.
	 * \param context Code generation context
	 * \param type The type of the instruction to be created
	 * \param data_type Data type associated with the operation
	 * \param destination Destination register of the instruction
	 * \param source Source register of the instruction
	 * \param base Base register for the memory operand
	 * \param index Index register for the memory operand
	 * \param scale Scale factor for the memory operand
	 * \param displacement Displacement for the memory operand
	 * \return Handle to the newly created instruction.
	 */
	handle<instruction> create_rrm_instruction(
		code_generator_context& context,
		instruction::type type,
		const data_type& data_type,
		i32 destination,
		i32 source,
		i32 base,
		i32 index,
		scale scale,
		i32 displacement
	);

	/**
	 * \brief Creates a register-memory (RM) instruction.
	 * \param context Code generation context
	 * \param type The type of the instruction to be created
	 * \param data_type Data type associated with the operation
	 * \param destination Destination register of the instruction
	 * \param base Base register for the memory operand
	 * \param index Index register for the memory operand
	 * \param scale Scale factor for the memory operand
	 * \param displacement Displacement for the memory operand
	 * \return Handle to the newly created instruction.
	 */
	handle<instruction> create_rm_instruction(
		code_generator_context& context,
		instruction::type type,
		const data_type& data_type,
		i32 destination,
		i32 base,
		i32 index,
		scale scale,
		i32 displacement
	);

	/**
	 * \brief Creates a memory-register (MR) instruction.
	 * \param context Code generation context
	 * \param type The type of the instruction to be created
	 * \param data_type Data type associated with the operation
	 * \param base Base register for the memory operand
	 * \param index Index register for the memory operand
	 * \param scale Scale factor for the memory operand
	 * \param displacement Displacement for the memory operand
	 * \param source Source register of the instruction
	 * \return Handle to the newly created instruction.
	 */
	handle<instruction> create_mr_instruction(
		code_generator_context& context,
		instruction::type type,
		const data_type& data_type,
		i32 base,
		i32 index,
		scale scale,
		i32 displacement,
		i32 source
	);

	/**
	 * \brief Creates a label instruction.
	 * \param context Code generation context
	 * \param n Associated block node
	 * \return Handle to the newly created instruction.
	 */
	handle<instruction> create_label_instruction(
		code_generator_context& context,
		handle<node> n
	);

	/**
	 * \brief Creates a mov instruction.
	 * \param context Code generation context
	 * \param data_type Data type associated with the operation
	 * \param destination Destination register ID
	 * \param source Source register ID
	 * \return Handle to the newly created instruction.
	 */
	handle<instruction> create_move_instruction(
		code_generator_context& context,
		const data_type& data_type,
		u8 destination,
		u8 source
	);

	/**
	 * \brief Creates an absolute immediate value.
	 * \param context Code generation context
	 * \param type The type of the instruction to be created
	 * \param data_type Data type associated with the operation
	 * \param destination Destination register
	 * \param imm The immediate value to be used
	 * \return Handle to the newly created instruction.
	 */
	handle<instruction> create_abs_instruction(
		code_generator_context& context,
		instruction::type type,
		const data_type& data_type,
		u8 destination,
		u64 imm
	);

	/**
	 * \brief Creates an instruction which sets the operand to0
	 * \param context Code generation context
	 * \param data_type Data type associated with the operation
	 * \param destination Destination register
	 * \return Handle to the newly created instruction.
	 */
	handle<instruction> create_zero_instruction(
		code_generator_context& context,
		const data_type& data_type,
		u8 destination
	);

	/**
	 * \brief Creates a 32-bit immediate instruction.
	 * \param context Code generation context
	 * \param type The type of the instruction to be created
	 * \param data_type Data type associated with the operation
	 * \param destination Destination register
	 * \param imm The immediate value to be used
	 * \return Handle to the newly created instruction.
	 */
	handle<instruction> create_immediate_instruction(
		code_generator_context& context,
		instruction::type type,
		const data_type& data_type,
		u8 destination,
		i32 imm
	);

	/**
	 * \brief Creates a register-register-immediate (RRI) instruction.
	 * \param context Code generation context
	 * \param type The type of the instruction to be created
	 * \param data_type Data type associated with the operation
	 * \param destination Destination register
	 * \param source Source register
	 * \param imm The immediate value to be used
	 * \return Handle to the newly created instruction.
	 */
	handle<instruction> create_rri_instruction(
		code_generator_context& context,
		instruction::type type,
		const data_type& data_type,
		u8 destination,
		u8 source,
		i32 imm
	);

	/**
	 * \brief Creates a register-register-register (RRR) instruction.
	 * \param context Code generation context
	 * \param type The type of the instruction to be created
	 * \param data_type Data type associated with the operation
	 * \param destination Destination register
	 * \param left Left register
	 * \param right Right register
	 * \return Handle to the newly created instruction.
	 */
	handle<instruction> create_rrr_instruction(
		code_generator_context& context,
		instruction::type type,
		const data_type& data_type,
		u8 destination,
		u8 left,
		u8 right
	);

	/**
	 * \brief Creates a jump (JMP) instruction.
	 * \param context Code generation context
	 * \param target Target block the instruction should jump to
	 * \return Handle to the newly created instruction.
	 */
	handle<instruction> create_jump_instruction(
		code_generator_context& context,
		handle<node> target
	);
}

