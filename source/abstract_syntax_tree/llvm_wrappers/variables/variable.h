#pragma once
#include "abstract_syntax_tree/llvm_wrappers/value.h"
#include <utility/filesystem/filesystem.h>

namespace sigma {
	/**
	 * \brief Base variable class. Contains the relevant llvm::Value* which refers to the variable declaration. 
	 */
	class variable {
	public:
		/**
		 * \brief Constructs the variable using the given \a value and \a position. 
		 * \param value llvm::Value* which refers to the variable declaration 
		 * \param range Range where the variable is declared at
		 */
		variable(
			value_ptr value,
			const utility::file_range& range
		);

		value_ptr get_value() const;
		const utility::file_range& get_range() const;
	private:
		value_ptr m_value;
		utility::file_range m_range;
	};

	using variable_ptr = s_ptr<variable>;
}
