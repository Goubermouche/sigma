#pragma once
#include "../type.h"

namespace channel {
	class value;
	using value_ptr = std::shared_ptr<value>;

	/**
	 * \brief Wrapper around llvm::Value*, provides utilities such as additional type information
	 */
	class value {
	public:
		value(const std::string& name, type type, llvm::Value* value);

		type get_type() const;
		llvm::Value* get_value() const;
		llvm::Value* get_pointer() const;
		void set_type(type ty);
		void set_value(llvm::Value* value);
		void set_pointer(llvm::Value* pointer);
		const std::string& get_name() const;
	private:
		std::string m_name;
		type m_type;
		llvm::Value* m_value;
		llvm::Value* m_pointer;
	};
}
