#include "code_generator_context.h"

namespace sigma {
	code_generator_context::code_generator_context()
		: m_builder(m_context),
	m_module(std::make_shared<llvm::Module>("sigma", m_context)) {}

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

	function_registry& code_generator_context::get_function_registry() {
		return m_function_registry;
	}

	variable_registry& code_generator_context::get_variable_registry() {
		return m_variable_registry;
	}

	outcome::result<void> code_generator_context::concatenate_function_registry(
		const function_registry& registry
	) {
		return m_function_registry.concatenate(registry);
	}

	outcome::result<void> code_generator_context::concatenate_variable_registry(
		const variable_registry& registry
	) {
		return m_variable_registry.concatenate(registry);
	}

	void code_generator_context::print() const {
		console::out << "---------------------------------------\n";
		m_module->print(llvm::outs(), nullptr);
		console::out << "---------------------------------------\n";

		m_variable_registry.print();
	}

	std::ostream& operator<<(
		std::ostream& stream, 
		const code_generator_context& context
	) {
		stream << "not implemented";
		return stream;
	}
}