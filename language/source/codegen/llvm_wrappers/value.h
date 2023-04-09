#pragma once
#include "../type.h"

namespace channel {
	/**
	 * \brief Wrapper around llvm::Value*, provides utilities such as additional type information
	 */
	class value {
	public:
		value(const std::string& name, type type, llvm::Value* value);

		type get_type() const;
		llvm::Value* get_value() const;
		void set_type(type ty);
		void set_value(llvm::Value* value);
		const std::string& get_name() const;
	private:
		std::string m_name;
		type m_type;
		llvm::Value* m_value;
	};
}
