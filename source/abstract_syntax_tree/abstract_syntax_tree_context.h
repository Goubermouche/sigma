#pragma once
#include <llvm/IR/IRBuilder.h>
#include "llvm_wrappers/functions/function_registry.h"
#include "llvm_wrappers/variables/variable_registry.h"

namespace sigma {
	/**
	 * \brief Code generator context, holds LLVM variables that are relevant to the code generation process. 
	 */
	class abstract_syntax_tree_context {
	public:
		abstract_syntax_tree_context();

		llvm::LLVMContext& get_context();
		llvm::IRBuilder<>& get_builder();
		ptr<llvm::Module> get_module();
		ptr<llvm::Module> get_module() const;

		function_registry& get_function_registry();
		variable_registry& get_variable_registry();

		utility::outcome::result<void> concatenate_function_registry(
			const function_registry& registry
		);

		utility::outcome::result<void> concatenate_variable_registry(
			const variable_registry& registry
		);

		void print() const;
	private:
		llvm::LLVMContext m_context;
		llvm::IRBuilder<> m_builder;
		ptr<llvm::Module> m_module;
		
		function_registry m_function_registry;
		variable_registry m_variable_registry;
	};
}
