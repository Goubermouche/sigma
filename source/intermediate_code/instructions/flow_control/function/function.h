#pragma once
#include "intermediate_code/instructions/flow_control/function/argument.h"
#include "intermediate_code/instructions/flow_control/block.h"
#include "intermediate_code/instructions/instruction.h"

namespace ir {
	class builder;

	class function;
	using function_ptr = ptr<function>;

	/**
	 * \brief Mid-level function object. Contains information about
	 * a function's arguments and return types. 
	 */
	class function : public value {
	public:
		function(
			type_ptr return_type,
			const std::vector<type_ptr>& argument_types,
			const std::string& name,
			builder& builder
		);


		const std::vector<function_argument_ptr>& get_arguments() const;
		const std::vector<block_ptr>& get_blocks() const;

		std::string to_string() override;
	private:
		void add_block(block_ptr block);
	private:
		friend class builder;

		type_ptr m_return_type;
		std::vector<function_argument_ptr> m_arguments;
		std::vector<block_ptr> m_blocks;
	};
}
