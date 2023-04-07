#pragma once
#include "../../type.h"

namespace channel {
	/**
	 * \brief Wrapper around llvm::Value*, provides utilities such as types
	 */
	class value {
	public:
		value(type type, llvm::Value* value);

		type get_type() const;
		llvm::Value* get_value() const;
	private:
		type m_type;
		llvm::Value* m_value;
	};
}
