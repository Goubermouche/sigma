#pragma once
#include "abstract_syntax_tree/llvm_wrappers/variables/variable.h"
#include <utility/diagnostics/error.h>

namespace sigma {
	class scope;
	using scope_ptr = ptr<scope>;

	/**
	 * \brief Base scope class, contains information about the current, parent, and child scopes,
	 * each scope contains a list of variables which have been defined there.
	 */
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
		 * \brief Concatenates \a this and \a other scopes. Does not change the \a other scope,
		 * updates the current scope.
		 * \param other Second scope to concatenate with. 
		 * \return Outcome (void) 
		 */
		utility::outcome::result<void> concatenate(
			const scope_ptr& other
		);

		/**
		 * \brief Inserts a new \a child scope. 
		 * \param child Scope to insert
		 */
		void add_child(
			const scope_ptr child
		);

		/**
		 * \brief Inserts a new \a variable, which will be accessible from this and all child scopes.
		 * \param identifier Identifier of the given \a variable 
		 * \param variable Variable to insert
		 * \return True if the insertion operation succeeded, false otherwise.
		 */
		bool insert_variable(
			const std::string& identifier,
			variable_ptr variable
		);

		/**
		 * \brief Checks if a named value exists in the scope.
		 * \param identifier Name of the named value to look for
		 * \return True if the named value exists.
		 */
		bool contains_variable(
			const std::string& identifier
		) const;

		/**
		 * \brief Returns the named value from the scope. If the named value does not exist, a nullptr is returned.
		 * \param identifier Name of the named value to look for
		 * \return Pointer to the value of our named value, may be nullptr if the value does not exist.
		 */
		variable_ptr get_variable(
			const std::string& identifier
		);

		/**
		 * \brief Returns the current loop end block.
		 * \return Current loop end block, may be nullptr if the loop end block has not been set.
		 */
		llvm::BasicBlock* get_loop_end_block() const;

		void print(u64 level = 0) const;
	private:
		scope_ptr m_parent = nullptr;
		std::vector<scope_ptr> m_children;

		llvm::BasicBlock* m_loop_end_block = nullptr;
		std::unordered_map<std::string, variable_ptr> m_variables;
	};
}
