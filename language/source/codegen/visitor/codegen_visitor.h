#pragma once

#include "../../parser/parser.h"
#include "../scope.h"
#include "visitor.h"

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
		codegen_visitor();

		bool generate(parser& parser);
		std::shared_ptr<llvm::Module> get_module();

		void print_intermediate_representation() const;
		bool verify_intermediate_representation() const;
	private:
		// functions
		// codegen_visitor_functions.cpp
		bool visit_function_node(function_node& node, value*& out_value) override;
		bool visit_function_call_node(function_call_node& node, value*& out_value) override;

		// variables
		// codegen_visitor_variables.cpp
		bool visit_assignment_node(assignment_node& node, value*& out_value) override;
		bool visit_variable_node(variable_node& node, value*& out_value) override;
		bool visit_local_declaration_node(local_declaration_node& node, value*& out_value) override;
		bool visit_global_declaration_node(global_declaration_node& node, value*& out_value) override;
		bool get_declaration_value(const declaration_node& node, value*& out_value); // utility

		// flow control
		// codegen_visitor_flow_control.cpp
		bool visit_return_node(return_node& node, value*& out_value) override;

		// types
		// codegen_visitor_types.cpp
		// signed integers
		bool visit_keyword_i8_node(i8_node& node, value*& out_value) override;
		bool visit_keyword_i16_node(i16_node& node, value*& out_value) override;
		bool visit_keyword_i32_node(i32_node& node, value*& out_value) override;
		bool visit_keyword_i64_node(i64_node& node, value*& out_value) override;
		// unsigned integers
		bool visit_keyword_u8_node(u8_node& node, value*& out_value) override;
		bool visit_keyword_u16_node(u16_node& node, value*& out_value) override;
		bool visit_keyword_u32_node(u32_node& node, value*& out_value) override;
		bool visit_keyword_u64_node(u64_node& node, value*& out_value) override;
		// floating point
		bool visit_keyword_f32_node(f32_node& node, value*& out_value) override;
		bool visit_keyword_f64_node(f64_node& node, value*& out_value) override;

		// operators
		// codegen_visitor_operators.cpp
		bool visit_operator_addition_node(operator_addition_node& node, value*& out_value) override;
		bool visit_operator_subtraction_node(operator_subtraction_node& node, value*& out_value) override;
		bool visit_operator_multiplication_node(operator_multiplication_node& node, value*& out_value) override;
		bool visit_operator_division_node(operator_division_node& node, value*& out_value) override;
		bool visit_operator_modulo_node(operator_modulo_node& node, value*& out_value) override;

		// utility
		llvm::Value* cast_value(const value* source_value, type target_type, u64 line_number);

		bool has_main_entry_point() const;
		void initialize_global_variables();
	private:
		// scope tree hierarchy
		scope* m_scope;

		// map of all global variables
		std::unordered_map<std::string, value*> m_global_named_values;

		// global initialization ctors
		std::vector<llvm::Constant*> m_global_ctors;

		// ctor initialization priority 
		i32 m_global_initialization_priority = 0;

		// list of all existing functions
		std::unordered_map<std::string, function*> m_functions;

		// llvm boilerplate
		llvm::LLVMContext m_context;
		llvm::IRBuilder<> m_builder;
		std::shared_ptr<llvm::Module> m_module;
	};
}