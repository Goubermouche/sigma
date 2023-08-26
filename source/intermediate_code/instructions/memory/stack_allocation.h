#pragma once
#include "intermediate_code/instructions/unary_instruction.h"
#include "intermediate_code/types/type.h"

namespace ir {
	class stack_allocation;
	using stack_allocation_instruction_ptr = ptr<stack_allocation>;

	/**
	 * \brief Stack allocation, contains information about a given allocation
	 * on the stack. 
	 */
	class stack_allocation : public unary_instruction {
	public:
		stack_allocation(
			value_ptr destination,
			type_ptr type,
			alignment alignment,
			const std::string& name
		);

		alignment get_alignment() const;

		std::string to_string() override;
	private:
		alignment m_alignment;
	};
}
