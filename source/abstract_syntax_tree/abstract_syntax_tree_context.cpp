#include "abstract_syntax_tree_context.h"

namespace sigma {
	abstract_syntax_tree_context::abstract_syntax_tree_context()
		: m_builder(m_context),
	m_module(std::make_shared<llvm::Module>("sigma", m_context)) {}

	llvm::LLVMContext& abstract_syntax_tree_context::get_context() {
		return m_context;
	}

	llvm::IRBuilder<>& abstract_syntax_tree_context::get_builder() {
		return m_builder;
	}

	s_ptr<llvm::Module> abstract_syntax_tree_context::get_module() {
		return m_module;
	}

	s_ptr<llvm::Module> abstract_syntax_tree_context::get_module() const {
		return m_module;
	}

	function_registry& abstract_syntax_tree_context::get_function_registry() {
		return m_function_registry;
	}

	variable_registry& abstract_syntax_tree_context::get_variable_registry() {
		return m_variable_registry;
	}

	utility::outcome::result<void> abstract_syntax_tree_context::concatenate_function_registry(
		const function_registry& registry
	) {
		return m_function_registry.concatenate(registry);
	}

	utility::outcome::result<void> abstract_syntax_tree_context::concatenate_variable_registry(
		const variable_registry& registry
	) {
		return m_variable_registry.concatenate(registry);
	}

	std::ostream& operator<<(
		std::ostream& stream, 
		const abstract_syntax_tree_context& context
	) {
		SUPPRESS_C4100(context);
		stream << "not implemented";
		return stream;
	}

	utility::console& operator<<(
		utility::console& console, 
		const abstract_syntax_tree_context& c
	) {
		utility::console::out << "---------------------------------------\n";
		c.m_module->print(llvm::outs(), nullptr);
		utility::console::out << "---------------------------------------\n";

		utility::console::out << c.m_variable_registry;
		return console;
	}
}