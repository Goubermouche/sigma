#pragma once
#include "llvm_wrappers/variables/scope.h"

namespace sigma {
	class variable_registry {
	public:
		variable_registry();
		void insert_local_variable(
			const std::string& identifier,
			variable_ptr variable
		);

		void insert_global_variable(
			const std::string& identifier,
			variable_ptr variable
		);

		variable_ptr get_variable(
			const std::string& identifier
		);

		variable_ptr get_local_variable(
			const std::string& identifier
		);

		variable_ptr get_global_variable(
			const std::string& identifier
		);

		bool contains_variable(
			const std::string& identifier
		);

		bool contains_global_variable(
			const std::string& identifier
		);

		void add_global_ctor(
			llvm::Constant* ctor
		);

		outcome::result<void> concatenate(
			const variable_registry& other
		);

		void push_scope();
		void pop_scope();
		void print() const;

		u64 increment_global_initialization_priority();
		const std::vector<llvm::Constant*>& get_global_ctors() const;
		u64 get_global_ctors_count();
		llvm::BasicBlock* get_loop_end_block();
	private:
		// local variables
		std::vector<scope_ptr> m_scopes;

		// global variables
		std::unordered_map<std::string, variable_ptr> m_global_variables;
		std::vector<llvm::Constant*> m_global_ctors;
		u64 m_initialization_priority = 0;
	};
}
