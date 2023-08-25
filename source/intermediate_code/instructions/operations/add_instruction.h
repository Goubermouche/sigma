#pragma once
#include "intermediate_code/instructions/binary_instruction.h"

namespace ir {
	class add_instruction;
	using add_instruction_ptr = ptr<add_instruction>;

	/**
	 * \brief Arithmetic add operation. Contains information about the
	 * operands and the destination of the operation.
	 */
	class add_instruction : public binary_instruction {
	public:
		add_instruction(
			value_ptr destination,
			value_ptr left_operand,
			value_ptr right_operand,
			const std::string& name
		);

		std::string to_string() override;
	};
}
