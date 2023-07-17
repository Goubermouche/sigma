#pragma once
#include <llvm/IR/IRBuilder.h>
#include "llvm_wrappers/functions/function_registry.h"
#include "llvm_wrappers/variables/variable_registry.h"

namespace sigma {
	/**
	 * \brief LLVM context, holds LLVM variables that are relevant to the code generation process. 
	 */
	class code_generator_context {
	public:
		code_generator_context();

		llvm::LLVMContext& get_context();
		llvm::IRBuilder<>& get_builder();
		std::shared_ptr<llvm::Module> get_module();
		std::shared_ptr<llvm::Module> get_module() const;

		function_registry& get_function_registry();
		variable_registry& get_variable_registry();

		void print() const;
	private:
		llvm::LLVMContext m_context;
		llvm::IRBuilder<> m_builder;
		std::shared_ptr<llvm::Module> m_module;
		
		function_registry m_function_registry;
		variable_registry m_variable_registry;
	};
}
