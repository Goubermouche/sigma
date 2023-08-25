#pragma once
#include "intermediate_code/instructions/instruction.h"
#include "intermediate_code/constants/constant.h"

namespace ir {
	class store_instruction;
	using store_instruction_ptr = ptr<store_instruction>;

	/**
	 * \brief Store instruction, contains information about a given store
	 * operation.
	 */
	class store_instruction : public instruction {
	public:
		store_instruction(
			value_ptr destination,
			constant_ptr value_to_store,
			const std::string& name
		);

		value_ptr get_value_to_store() const;

		std::string to_string() override;
	protected:
		constant_ptr m_value_to_store;
	};
}

