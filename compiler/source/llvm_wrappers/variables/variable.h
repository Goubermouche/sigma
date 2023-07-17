#pragma once
#include "llvm_wrappers/value.h"

namespace sigma {
	/**
	 * \brief Base variable class. Contains the relevant llvm::Value* which refers to the variable declaration. 
	 */
	class variable {
	public:
		/**
		 * \brief Constructs the variable using the given \a value and \a position. 
		 * \param value llvm::Value* which refers to the variable declaration 
		 * \param position Position which the variable is declared at
		 */
		variable(
			value_ptr value,
			file_position position
		);

		value_ptr get_value() const;
		const file_position& get_position() const;
	private:
		value_ptr m_value;
		file_position m_position;
	};

	using variable_ptr = std::shared_ptr<variable>;
}
