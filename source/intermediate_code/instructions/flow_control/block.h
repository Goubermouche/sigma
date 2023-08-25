#pragma once
#include "intermediate_code/instructions/instruction.h"

namespace ir {
	class block;
	using block_ptr = ptr<block>;

	/**
	 * \brief Basic block class. Contains a list of instructions. 
	 */
	class block : public value {
	public:
		block(const std::string& name);

		template<typename type>
		ptr<type> append(ptr<type> instruction);

		const std::vector<instruction_ptr>& get_instructions() const;

		std::string to_string() override;
	private:
		// contained instructions 
		std::vector<instruction_ptr> m_instructions;
	};

	template<typename type>
	ptr<type> block::append(ptr<type> instruction) {
		m_instructions.emplace_back(instruction);
		return instruction;
	}
}
