#include "codegen_visitor.h"

namespace channel {
	codegen_visitor::codegen_visitor() : m_builder(m_context) {
		m_module = std::make_unique<llvm::Module>("channel", m_context);
	}

	llvm::Value* codegen_visitor::visit_keyword_i32_node(keyword_i32_node& node) {
		return nullptr;
	}

	llvm::Value* codegen_visitor::visit_operator_addition_node(operator_addition_node& node) {
		//llvm::Value* left = node.left->accept(*this);
		//llvm::Value* right = node.right->accept(*this);
		//return m_builder.CreateAdd(left, right, "addtmp");
		return nullptr;
	}
}