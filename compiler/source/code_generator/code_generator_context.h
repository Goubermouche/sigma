#pragma once
#include <llvm/IR/IRBuilder.h>
#include "llvm_wrappers/scope.h"
#include "llvm_wrappers/functions/function_registry.h"
#include "llvm_wrappers/variables/global_variable_registry.h"

namespace sigma {
	/**
	 * \brief LLVM context, holds LLVM variables that are relevant to the code generation process. 
	 */
	class code_generator_context {
	public:
		code_generator_context();
		void print_intermediate_representation() const;

		llvm::LLVMContext& get_context();
		llvm::IRBuilder<>& get_builder();
		std::shared_ptr<llvm::Module> get_module();
		std::shared_ptr<llvm::Module> get_module() const;

		scope_ptr get_scope();
		function_registry& get_function_registry();
		global_variable_registry& get_global_variable_registry();
	private:
		llvm::LLVMContext m_context;
		llvm::IRBuilder<> m_builder;
		std::shared_ptr<llvm::Module> m_module;

		scope_ptr m_scope;
		function_registry m_function_registry;
		global_variable_registry m_global_variable_registry;
	};
}
