#pragma once

#include "../utility/macros.h"
#include "visitor.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>

namespace channel {
	class declaration_node;

	/**
	 * \brief Evaluator that implements the codegen visitor to generate LLVM IR.
	 */
	class codegen_visitor : public visitor {
	public:
		codegen_visitor();

		void print_intermediate_representation() const;
		void verify_intermediate_representation() const;
	private:
		// variables
		llvm::Value* visit_assignment_node(assignment_node& node) override;
		llvm::Value* visit_function_call_node(function_call_node& node) override;
		llvm::Value* visit_variable_node(variable_node& node) override;
		llvm::Value* visit_function_node(function_node& node) override;

		llvm::Value* visit_local_declaration_node(local_declaration_node& node) override;
		llvm::Value* visit_global_declaration_node(global_declaration_node& node) override;

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
		llvm::Value* find_variable(const std::string& name);
		llvm::Value* get_declaration_value(const declaration_node& node);
	private:
		// stack holding all variables of each respective scope
		std::vector<std::unordered_map<std::string, llvm::Value*>> m_scope_stack;

		// map of all global variables
		std::unordered_map<std::string, llvm::Value*> m_global_named_values;

		llvm::LLVMContext m_context;
		llvm::IRBuilder<> m_builder;
		std::unique_ptr<llvm::Module> m_module;
	};
}