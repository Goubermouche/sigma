#pragma once
#include <llvm/IR/IRBuilder.h>

namespace channel {
	/**
	 * \brief LLVM context, holds LLVM variables that are relevant to the code generation process. 
	 */
	class llvm_context {
	public:
		llvm_context();

		llvm::LLVMContext& get_context();
		llvm::IRBuilder<>& get_builder();
		std::shared_ptr<llvm::Module> get_module();
		std::shared_ptr<llvm::Module> get_module() const;
	private:
		llvm::LLVMContext m_context;
		llvm::IRBuilder<> m_builder;
		std::shared_ptr<llvm::Module> m_module;
	};
}
