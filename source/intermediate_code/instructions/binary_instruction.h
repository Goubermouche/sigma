#pragma once
#include "intermediate_code/instructions/instruction.h"

namespace ir {
	class binary_instruction;
	using binary_instruction_ptr = ptr<binary_instruction>;

	/**
	 * \brief Base binary instruction class. Contains information about two
	 * operands and the destination.
	 */
	class binary_instruction : public instruction {
	public:
		binary_instruction(
			value_ptr destination,
			value_ptr left_operand,
			value_ptr right_operand,
			const std::string& name
		);

		value_ptr get_left_operand() const;
		value_ptr get_right_operand() const;
	protected:
		value_ptr m_left_operand;
		value_ptr m_right_operand;
	};
}