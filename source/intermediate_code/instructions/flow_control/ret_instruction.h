#pragma once
#include "intermediate_code/instructions/instruction.h"

namespace ir {
	class ret_instruction;
	using ret_instruction_ptr = ptr<ret_instruction>;

	/**
	 * \brief Return instruction. Contains information about the return value.
	 */
	class ret_instruction : public instruction {
	public:
		/**
		 * \brief Constructs a void (empty) return instruction
		 */
		ret_instruction();

		/**
		 * \brief Constructs a non-void (not-empty) return instruction with the
		 * given \a return_value.
		 * \param return_value Value to return
		 */
		ret_instruction(
			value_ptr return_value
		);

		value_ptr get_return_value() const;

		std::string to_string() override;
	private:
		value_ptr m_return_value;
	};
}