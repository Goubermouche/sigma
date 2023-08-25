#pragma once
#include "intermediate_code/value.h"

namespace ir {
	class instruction;
	using instruction_ptr = ptr<instruction>;

	/**
	 * \brief Base instruction class type. Represents a three address code (TAC)
	 * instruction (for more information see unary_instruction and binary_instruction)
	 */
	class instruction : public value {
	public:
		instruction(value_ptr destination, const std::string& name);

		value_ptr get_destination() const;
		std::string get_value_string() override;
	protected:
		value_ptr m_destination;
	};
}
