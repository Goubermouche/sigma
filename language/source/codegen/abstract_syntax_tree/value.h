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
		void set_type(type ty);
		void set_value(llvm::Value* value);
	private:
		type m_type;
		llvm::Value* m_value;
	};

	class function {
	public:
		function(type return_type, llvm::Function* function);

		type get_return_type() const;
		llvm::Function* get_function() const;
	private:
		type m_return_type;
		llvm::Function* m_value;
	};
}
