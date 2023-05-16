#pragma once
#include <llvm/IR/IRBuilder.h>

namespace channel {
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
