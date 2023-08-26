#pragma once
#include "intermediate_code/instructions/unary_instruction.h"
#include "intermediate_code/constants/constant.h"

namespace ir {
	class load_instruction;
	using load_instruction_ptr = ptr<load_instruction>;

	/**
	 * \brief Load instruction. Contains information about the address, alignment and
	 * type of the value which is being loaded 
	 */
	class load_instruction : public unary_instruction {
	public:
		load_instruction(
			value_ptr destination,
			value_ptr value_to_load,
			type_ptr value_type,
			alignment alignment,
			const std::string& name
		);

		alignment get_alignment() const;
		type_ptr get_value_type() const;

		std::string to_string() override;
	protected:
		alignment m_alignment;
		type_ptr m_value_type;
	};
}
