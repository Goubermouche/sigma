#pragma once
#include "visitor.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>

namespace channel {
	/**
	 * \brief Evaluator that implements the codegen visitor to generate LLVM IR.
	 */
	class codegen_visitor : public visitor {
	public:
		codegen_visitor();

		// keywords
		llvm::Value* visit_keyword_i32_node(keyword_i32_node& node) override;

		// operators
		llvm::Value* visit_operator_addition_node(operator_addition_node& node) override;
	private:
		llvm::LLVMContext m_context;
		llvm::IRBuilder<> m_builder;
		std::unique_ptr<llvm::Module> m_module;
	};
}