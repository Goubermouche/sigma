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
		codegen_visitor(const parser& parser);

		bool generate();
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
		bool visit_access_node(access_node& node, value*& out_value) override;
		bool visit_local_declaration_node(local_declaration_node& node, value*& out_value) override;
		bool visit_global_declaration_node(global_declaration_node& node, value*& out_value) override;
		bool visit_allocation_node(array_allocation_node& node, value*& out_value) override;
		bool visit_array_access_node(array_access_node& node, value*& out_value) override;
		bool visit_array_assignment_node(array_assignment_node& node, value*& out_value) override;
		bool visit_variable_node(variable_node& node, value*& out_value) override;

		// utility
		bool get_declaration_value(const declaration_node& node, value*& out_value); 
		bool get_named_value(value*& out_value, const std::string& variable_name);

		// flow control
		// codegen_visitor_flow_control.cpp
		bool visit_return_node(return_node& node, value*& out_value) override;
		bool visit_if_else_node(if_else_node& node, value*& out_value) override;

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
		// text
		bool visit_keyword_char_node(char_node& node, value*& out_value) override;
		bool visit_keyword_string_node(string_node& node, value*& out_value) override;
		// other
		bool visit_keyword_bool_node(bool_node& node, value*& out_value) override;

		// operators
		// codegen_visitor_operators.cpp
		// unary
		// arithmetic
		bool visit_operator_post_decrement_node(operator_post_decrement& node, value*& out_value) override;
		bool visit_operator_post_increment_node(operator_post_increment& node, value*& out_value) override;
		bool visit_operator_pre_decrement_node(operator_pre_decrement& node, value*& out_value) override;
		bool visit_operator_pre_increment_node(operator_pre_increment& node, value*& out_value) override;
		// binary
		// arithmetic
		bool visit_operator_addition_node(operator_addition_node& node, value*& out_value) override;
		bool visit_operator_subtraction_node(operator_subtraction_node& node, value*& out_value) override;
		bool visit_operator_multiplication_node(operator_multiplication_node& node, value*& out_value) override;
		bool visit_operator_division_node(operator_division_node& node, value*& out_value) override;
		bool visit_operator_modulo_node(operator_modulo_node& node, value*& out_value) override;
		// logical
		bool visit_operator_logical_conjunction_node(operator_conjunction_node& node, value*& out_value) override;
		bool visit_operator_logical_disjunction_node(operator_disjunction_node& node, value*& out_value) override;
		bool visit_operator_greater_than_node(operator_greater_than_node& node, value*& out_value) override;
		bool visit_operator_greater_than_equal_to(operator_greater_than_equal_to_node& node, value*& out_value) override;
		bool visit_operator_less_than_node(operator_less_than_node& node, value*& out_value) override;
		bool visit_operator_less_than_equal_to_node(operator_less_than_equal_to_node& node, value*& out_value) override;
		bool visit_operator_equals_node(operator_equals_node& node, value*& out_value) override;
		bool visit_operator_not_equals_node(operator_not_equals_node& node, value*& out_value) override;

		// utility
		bool cast_value(llvm::Value*& out_value, const value* source_value, type target_type, u64 line_number);

		bool has_main_entry_point() const;
		void initialize_global_variables();
	private:
		parser m_parser;

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