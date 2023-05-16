#include "code_generator.h"

namespace channel {
    code_generator::code_generator()
	    : m_builder(m_context),
        m_module(std::make_unique<llvm::Module>("channel", m_context)) {}

    void code_generator::set_abstract_syntax_tree(std::shared_ptr<abstract_syntax_tree> abstract_syntax_tree) {
        m_abstract_syntax_tree = abstract_syntax_tree;
    }

    std::shared_ptr<llvm::Module> code_generator::get_llvm_module() const {
        return m_module;
    }
}