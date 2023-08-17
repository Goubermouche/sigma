#pragma once
#include "llvm_wrappers/variables/scope.h"

namespace sigma {
	/**
	 * \brief Base variable registry class. Contains information about variables
	 * that have been declared in a certain compilation unit. 
	 */
	class variable_registry {
	public:
		variable_registry();

		/**
		 * \brief Inserts a new local \a variable under the given \a identifier. 
		 * \param identifier Identifier to insert the \a variable under
		 * \param variable Variable to insert
		 * \return True if the insertion operation succeeded, false otherwise.
		 */
		bool insert_local_variable(
			const std::string& identifier,
			variable_ptr variable
		);

		/**
		 * \brief Inserts a new global \a variable under the given \a identifier.
		 * \param identifier Identifier to insert the \a variable under
		 * \param variable Variable to insert
		 * \return True if the insertion operation succeeded, false otherwise.
		 */
		bool insert_global_variable(
			const std::string& identifier,
			variable_ptr variable
		);

		/**
		 * \brief Attempts to get the \a variable that has been declared under the specified \a identifier.
		 * \param identifier Variable identifier to search for
		 * \return variable_ptr if there is a valid variable, otherwise nullptr
		 */
		variable_ptr get_variable(
			const std::string& identifier
		);

		/**
		 * \brief Attempts to get the local \a variable that has been declared under the specified \a identifier.
		 * \param identifier Variable identifier to search for
		 * \return variable_ptr if there is a valid variable, otherwise nullptr
		 */
		variable_ptr get_local_variable(
			const std::string& identifier
		);

		/**
		 * \brief Attempts to get the global \a variable that has been declared under the specified \a identifier.
		 * \param identifier Variable identifier to search for
		 * \return variable_ptr if there is a valid variable, otherwise nullptr
		 */
		variable_ptr get_global_variable(
			const std::string& identifier
		);

		/**
		 * \brief Checks if the registry contains a variable that has been declared under the specified \a identifier
		 * (searches for global and local variables).
		 * \param identifier Identifier to look for
		 * \return True if the registry contains the queried variable identifier, false otherwise
		 */
		bool contains_variable(
			const std::string& identifier
		);

		/**
		 * \brief Checks if the registry contains a global variable that has been declared under the specified \a identifier.
		 * \param identifier Identifier to look for
		 * \return True if the registry contains the queried global variable identifier, false otherwise
		 */
		bool contains_global_variable(
			const std::string& identifier
		);

		/**
		 * \brief Inserts a new global ctor. 
		 * \param ctor Ctor to insert
		 */
		void add_global_ctor(
			llvm::Constant* ctor
		);
		/**
		 * \brief Concatenates \a this and \a other registries. Does not change the \a other registry,
		 * updates the current registry.
		 * \param other Second registry to concatenate with.
		 * \return Outcome (void)
		 */
		outcome::result<void> concatenate(
			const variable_registry& other
		);

		/**
		 * \brief Pushes a new local variable scope.
		 */
		void push_scope();

		/**
		 * \brief Pops the last local variable scope.
		 */
		void pop_scope();

		void print() const;

		u64 get_global_variable_count() const;

		llvm::BasicBlock* get_loop_end_block() const;
	private:
		// local variables
		std::vector<scope_ptr> m_scopes;

		// global variables
		std::unordered_map<std::string, variable_ptr> m_global_variables;
	};
}
