#pragma once
#include "llvm_wrappers/value.h"

namespace sigma {
	class scope;
	using scope_ptr = std::shared_ptr<scope>;

	class scope	{
	public:
		/**
		 * \brief Constructs the scope using the given parent scope and the loop end block.
		 * \param parent Enclosing scope in which this scope is defined
		 */
		scope(
			scope_ptr parent
		);

		/**
		 * \brief Constructs the scope using the given parent scope and the loop end block.
		 * \param parent Enclosing scope in which this scope is defined
		 * \param loop_end_block Basic LLVM block where the current loop ends
		 */
		scope(
			scope_ptr parent,
			llvm::BasicBlock* loop_end_block
		);

		/**
		 * \brief Inserts a new named value into the scope.
		 * \param name Name of the value
		 * \param value Value to insert
		 */
		void insert_named_value(
			const std::string& name, 
			value_ptr value
		);

		/**
		 * \brief Inserts a new named value into the scope.
		 * \param name Name of the value
		 * \param value Value to insert
		 * \return Pair, where the first element is an iterator to the added element, and the second element is a boolean that is true if the insertion took place, and false if the value already existed.
		 */
		std::pair<std::unordered_map<std::string, value_ptr>::iterator, bool> add_named_value(
			const std::string& name,
			value_ptr value
		);

		/**
		 * \brief Checks if a named value exists in the scope.
		 * \param name Name of the named value to look for
		 * \return True if the named value exists.
		 */
		bool contains_named_value(
			const std::string& name
		) const;

		/**
		 * \brief Returns the named value from the scope. If the named value does not exist, a nullptr is returned.
		 * \param name Name of the named value to look for
		 * \return Pointer to the value of our named value, may be nullptr if the value does not exist.
		 */
		value_ptr get_named_value(
			const std::string& name
		);

		/**
		 * \brief Returns the current loop end block.
		 * \return Current loop end block, may be nullptr if the loop end block has not been set.
		 */
		llvm::BasicBlock* get_loop_end_block() const;
	private:
		scope_ptr m_parent = nullptr;
		llvm::BasicBlock* m_loop_end_block = nullptr;
		std::unordered_map<std::string, value_ptr> m_named_values;
	};
}
