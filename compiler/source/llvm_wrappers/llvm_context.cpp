#include "llvm_context.h"

namespace channel {
	llvm_context::llvm_context()
		: m_builder(m_context),
	m_module(std::make_unique<llvm::Module>("channel", m_context)) {}

	llvm::LLVMContext& llvm_context::get_context() {
		return m_context;
	}

	llvm::IRBuilder<>& llvm_context::get_builder() {
		return m_builder;
	}

	std::shared_ptr<llvm::Module> llvm_context::get_module() {
		return m_module;
	}

	std::shared_ptr<llvm::Module> llvm_context::get_module() const {
		return m_module;
	}
}