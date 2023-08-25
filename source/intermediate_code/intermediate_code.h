// Root intermediate code file, contains the main builder class which functions as a general
// facade during the process of generating an intermediate representation of a given AST.

#pragma once
#include "intermediate_code/types/integer_type.h"
#include "intermediate_code/constants/integer_constant.h"

// instructions
#include "intermediate_code/instructions/flow_control/ret_instruction.h"
#include "intermediate_code/instructions/flow_control/function/function.h"
#include "intermediate_code/instructions/memory/stack_allocation.h"
#include "intermediate_code/instructions/memory/store_instruction.h"
#include "intermediate_code/instructions/operations/add_instruction.h"

using namespace utility::types;

namespace ir {
	/**
	 * \brief Basic IR builder. Creates a facade around the process of creating a TAC based
	 * intermediate representation of a given higher-level AST. The generated intermediate
	 * representation can be either printed or forwarded to a code generator, which can then
	 * export the related machine code for the given platform, and emit an executable.
	 */
	class builder {
	public:
		// todo: add support for specifying alignment

		/**
		 * \brief Creates a new stack allocation instruction, with the given alignment .
		 * \param type Type of the value which will by allocated (see ir::type)
		 * \param name Optional debug name of the operation
		 * \return Result of the stack allocation, which can be further used (ie. for
		 * storing values in it).
		 */
		stack_allocation_instruction_ptr create_stack_allocation(
			type_ptr type,
			const std::string& name = ""
		);

		/**
		 * \brief Creates a new store instruction. 
		 * \param allocation Allocation to store \a value_to_store at
		 * \param value_to_store Value to store
		 * \param name Optional debug name of the operation
		 * \return Result of the store operation, which can be further used.
		 */
		store_instruction_ptr create_store(
			stack_allocation_instruction_ptr allocation,
			constant_ptr value_to_store,
			const std::string& name = ""
		);

		/**
		 * \brief Creates a new add instruction (arithmetic).
		 * \param left Left operand of the add instruction
		 * \param right Right operand of the add instruction
		 * \param name Optional debug name of the operation
		 * \return Result of the add operation, which can be further used
		 */
		add_instruction_ptr create_add(
			value_ptr left,
			value_ptr right,
			const std::string& name = ""
		);

		/**
		 * \brief Creates a new function.
		 * \param return_type Return type of the function (see ir::type)
		 * \param argument_types List of types specifying the expected function arguments 
		 * \param name Optional debug name of the operation
		 * \return Pointer to the function which can be used later (ie. for creating blocks)
		 */
		function_ptr create_function(
			type_ptr return_type,
			const std::vector<type_ptr>& argument_types,
			const std::string& name = ""
		);

		/**
		 * \brief Creates a new ret instruction. Note that the \a return_value type must be
		 * the same as the return value of the parent function.
		 * \param return_value Value to return
		 * \return Return instruction pointer.
		 */
		ret_instruction_ptr create_ret(
			value_ptr return_value
		);

		/**
		 * \brief Creates a new void (empty) ret instruction. Note that the parent function's
		 * return type must also be of type void (empty)
		 * \return Void return instruction pointer.
		 */
		ret_instruction_ptr create_ret_void();

		/**
		 * \brief Creates a new block inside the given \a parent function (appends it to the
		 * list of contained blocks). 
		 * \param parent_function Function to insert the block into
		 * \param name Optional debug name of the operation
		 * \return Block pointer, which can be used in the \a set_insert_point method.
		 */
		block_ptr create_block(
			function_ptr parent_function,
			const std::string& name = ""
		);

		/**
		 * \brief Sets the new insert point for the builder. This means that whenever a method,
		 * which would, in any way, shape or form add a new instruction, that instruction will be
		 * appended into the specified \a block. 
		 * \param block Block to set as the new insert point
		 */
		void set_insert_point(
			block_ptr block
		);

		/**
		 * \brief Prints the contents of the builder into the console.
		 */
		void print() const;
	private:
		/**
		 * \brief Utility method - checks whether we have a valid insert point, and if we do,
		 * appends the given \a instruction to it, otherwise breaks. 
		 * \tparam type Type of the instruction, this should derive from ir::value
		 * \param instruction Instruction to append
		 * \return Value which was passed as \a instruction, without any changes.
		 */
		template<typename type>
		ptr<type> append(
			ptr<type> instruction
		);

		/**
		 * \brief Utility method - creates a new debug name. If name is an empty string a static
		 * counter is incremented and its value is used as the new debug name. 
		 * \param name Name to use as a debug name. Can be an empty string
		 * \return Debug name - either the contents of \a name, or a number ranging from 0 to u64::max.
		 */
		std::string create_debug_name(const std::string& name) const;
	private:
		friend class function;

		std::vector<function_ptr> m_functions;
		block_ptr m_insert_point;
	};

	template<typename type>
	ptr<type> builder::append(
		ptr<type> instruction
	) {
		ASSERT(m_insert_point != nullptr, "insert point not specified");
		return m_insert_point->append(instruction);
	}
}