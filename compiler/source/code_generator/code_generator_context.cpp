#include "code_generator_context.h"

namespace sigma {
	code_generator_context::code_generator_context()
		: m_builder(m_context),
	m_module(std::make_unique<llvm::Module>("sigma", m_context)),
	m_scope(new scope(nullptr, nullptr)) {}

	void code_generator_context::print_intermediate_representation() const {
		m_module->print(llvm::outs(), nullptr);
	}

	llvm::LLVMContext& code_generator_context::get_context() {
		return m_context;
	}

	llvm::IRBuilder<>& code_generator_context::get_builder() {
		return m_builder;
	}

	std::shared_ptr<llvm::Module> code_generator_context::get_module() {
		return m_module;
	}

	std::shared_ptr<llvm::Module> code_generator_context::get_module() const {
		return m_module;
	}

	scope_ptr code_generator_context::get_scope() {
		return m_scope;
	}

	function_registry& code_generator_context::get_function_registry() {
		return m_function_registry;
	}

	global_variable_registry& code_generator_context::get_global_variable_registry() {
		return m_global_variable_registry;
	}
}