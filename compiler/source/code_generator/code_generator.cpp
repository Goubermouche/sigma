#include "code_generator.h"

namespace channel {
	code_generator::code_generator()
		: m_llvm_context(std::make_shared<llvm_context>()) {} 

	void code_generator::set_abstract_syntax_tree(std::shared_ptr<abstract_syntax_tree> abstract_syntax_tree) {
		m_abstract_syntax_tree = abstract_syntax_tree;
	}

	std::shared_ptr<llvm_context> code_generator::get_llvm_context() {
		return m_llvm_context;
	}
}