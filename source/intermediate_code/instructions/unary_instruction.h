#pragma once
#include "intermediate_code/instructions/instruction.h"

namespace ir {
	class unary_instruction;
	using unary_instruction_ptr = ptr<unary_instruction>;

	/**
	 * \brief Base unary instruction class. Contains information about one
	 * operand and the destination.
	 */
	class unary_instruction : public instruction {
	public:
		unary_instruction(
			value_ptr destination,
			value_ptr operand,
			const std::string& name
		);

		value_ptr get_operand() const;
	protected:
		value_ptr m_operand;
	};
}