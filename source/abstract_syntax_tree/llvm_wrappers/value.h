#pragma once
#include "abstract_syntax_tree/llvm_wrappers/type.h"

namespace sigma {
	class value;
	using value_ptr = std::shared_ptr<value>;

	/**
	 * \brief Wrapper around an llvm::Value*, provides utilities such as additional type information
	 */
	class value {
	public:
		/**
		 * \brief Constructs a new value. 
		 * \param name Name of the value
		 * \param type Type of the value
		 * \param value LLVM value* of the value
		 */
		value(
			const std::string& name,
			type type, 
			llvm::Value* value
		);

		/**
		 * \brief Gets the value type.
		 * \return Type of the value.
		 */
		type get_type() const;

		/**
		 * \brief Gets the inherent value of the value.
		 * \return LLVM value* representing the value blocks.
		 */
		llvm::Value* get_value() const;

		/**
		 * \brief Gets an LLVM value* pointing to the store location of this value.
		 * \return LLVM value* pointing to the store location of this value.
		 */
		llvm::Value* get_pointer() const;

		/**
		 * \brief Sets the new value type of the value.
		 * \param ty Type to use
		 */
		void set_type(
			type ty
		);

		/**
		 * \brief Sets the new inherent value of the value.
		 * \param value Value to use
		 */
		void set_value(
			llvm::Value* value
		);

		/**
		 * \brief Sets the new pointer to the store of the value.
		 * \param pointer Pointer to use
		 */
		void set_pointer(
			llvm::Value* pointer
		);

		/**
		 * \brief Gets the name of the value.
		 * \return Name of the value.
		 */
		const std::string& get_name() const;
	private:
		std::string m_name;
		type m_type;
		llvm::Value* m_value;
		llvm::Value* m_pointer;
	};
}
