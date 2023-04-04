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

		void print_code() const;
		void verify() const;
	private:
		// variables
		llvm::Value* visit_assignment_node(assignment_node& node) override;
		llvm::Value* visit_declaration_node(declaration_node& node) override;
		llvm::Value* visit_function_call_node(function_call_node& node) override;
		llvm::Value* visit_variable_node(variable_node& node) override;
		llvm::Value* visit_function_node(function_node& node) override;

		// keywords
		llvm::Value* visit_keyword_i8_node(keyword_i8_node& node) override;
		llvm::Value* visit_keyword_i16_node(keyword_i16_node& node) override;
		llvm::Value* visit_keyword_i32_node(keyword_i32_node& node) override;
		llvm::Value* visit_keyword_i64_node(keyword_i64_node& node) override;

		// operators
		llvm::Value* visit_operator_addition_node(operator_addition_node& node) override;
		llvm::Value* visit_operator_subtraction_node(operator_subtraction_node& node) override;
		llvm::Value* visit_operator_multiplication_node(operator_multiplication_node& node) override;
		llvm::Value* visit_operator_division_node(operator_division_node& node) override;
		llvm::Value* visit_operator_modulo_node(operator_modulo_node& node) override;

		bool has_main_entry_point() const;
	private:
		llvm::LLVMContext m_context;
		llvm::IRBuilder<> m_builder;
		std::unordered_map<std::string, llvm::Value*> m_named_values;
		std::unordered_map<std::string, llvm::Value*> m_global_named_values;
	public:
		std::unique_ptr<llvm::Module> m_module;
	};
}