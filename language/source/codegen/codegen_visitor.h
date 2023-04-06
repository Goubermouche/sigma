#pragma once

#include "scope.h"
#include "visitor.h"
#include "../parser/parser.h"

#include <llvm/IR/IRBuilder.h>

#define GLOBAL_INITIALIZATION_FUNCTION "__global_init_function"

#define CTOR_STRUCT_TYPE                 \
llvm::StructType::get(m_context, {       \
	llvm::Type::getInt32Ty(m_context),   \
	llvm::Type::getInt8PtrTy(m_context), \
	llvm::Type::getInt8PtrTy(m_context)  \
})                                      

namespace channel {
	class declaration_node;

	/**
	 * \brief Evaluator that implements the codegen visitor to generate LLVM IR.
	 */
	class codegen_visitor : public visitor {
	public:
		codegen_visitor(parser& parser);

		void print_intermediate_representation() const;
		void verify_intermediate_representation() const;
	private:
		// variables
		llvm::Value* visit_assignment_node(assignment_node& node) override;
		llvm::Value* visit_function_call_node(function_call_node& node) override;
		llvm::Value* visit_variable_node(variable_node& node) override;
		llvm::Value* visit_function_node(function_node& node) override;
		llvm::Value* visit_return_node(return_node& node) override;

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
		void initialize_global_variables();
		llvm::Type* type_to_llvm_type(type type);
		llvm::Value* get_declaration_value(const declaration_node& node);
	private:
		// scope tree hierarchy
		scope* m_scope;

		// map of all global variables
		std::unordered_map<std::string, llvm::Value*> m_global_named_values;
		// global initialization ctors
		std::vector<llvm::Constant*> m_ctors;
		// ctor initialization priority 
		i32 m_global_initialization_priority = 0;

		std::unordered_map<std::string, llvm::Function*> m_functions;

		// special functions
		llvm::LLVMContext m_context;
		llvm::IRBuilder<> m_builder;
		std::unique_ptr<llvm::Module> m_module;
	};
}