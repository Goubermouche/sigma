#pragma once

#include "parser/parser.h"
#include "code_generator/code_generator.h"
#include "llvm_wrappers/scope.h"
#include "llvm_wrappers/code_generation_context.h"
#include "llvm_wrappers/functions/function_registry.h"

#define CTOR_STRUCT_TYPE                                     \
llvm::StructType::get(m_llvm_context->get_context(), {       \
	llvm::Type::getInt32Ty(m_llvm_context->get_context()),   \
	llvm::Type::getInt8PtrTy(m_llvm_context->get_context()), \
	llvm::Type::getInt8PtrTy(m_llvm_context->get_context())  \
})

namespace sigma {
	class declaration_node;

	/**
	 * \brief Evaluator that implements the codegen visitor to generate LLVM IR.
	 */
	class basic_code_generator : public code_generator {
	public:
		basic_code_generator();

		outcome::result<void> generate() override;
		outcome::result<void> verify_intermediate_representation();
	private:
		// functions
		// codegen_visitor_functions.cpp
		outcome::result<value_ptr> visit_function_node(
			function_node& node, 
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_function_call_node(
			function_call_node& node,
			const code_generation_context& context
		) override;

		// keywords
		// codegen_visitor_keywords.cpp
		virtual outcome::result<value_ptr> visit_file_include_node(
			file_include_node& node,
			const code_generation_context& context
		) override;

		// variables
		// codegen_visitor_variables.cpp
		outcome::result<value_ptr> visit_assignment_node(
			assignment_node& node,
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_variable_access_node(
			variable_access_node& node, 
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_local_declaration_node(
			local_declaration_node& node,
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_global_declaration_node(
			global_declaration_node& node,
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_allocation_node(
			array_allocation_node& node, 
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_array_access_node(
			array_access_node& node,
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_array_assignment_node(
			array_assignment_node& node, 
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_variable_node(
			variable_node& node, 
			const code_generation_context& context
		) override;

		// utility
		outcome::result<value_ptr> get_declaration_value(
			const declaration_node& node,
			const code_generation_context& context
		);

		/**
		 * \brief Creates LLVM blocks for the \a add operation.
		 * \param left_operand Left operand
		 * \param right_operand Right operand
		 * \return Expected - value(operation result, operation precision, left operand result), error received from inner accept invocations.
		 */
		outcome::result<std::tuple<llvm::Value*, type, value_ptr>> create_add_operation(
			node* left_operand,
			node* right_operand
		);

		/**
		 * \brief Creates LLVM blocks for the \a sub operation.
		 * \param left_operand Left operand
		 * \param right_operand Right operand
		 * \return Expected - value(operation result, operation precision, left operand result), error received from inner accept invocations.
		 */
		outcome::result<std::tuple<llvm::Value*, type, value_ptr>> create_sub_operation(
			node* left_operand,
			node* right_operand
		);

		/**
		 * \brief Creates LLVM blocks for the \a mul operation.
		 * \param left_operand Left operand
		 * \param right_operand Right operand
		 * \return Expected - value(operation result, operation precision, left operand result), error received from inner accept invocations.
		 */
		outcome::result<std::tuple<llvm::Value*, type, value_ptr>> create_mul_operation(
			node* left_operand,
			node* right_operand
		);

		/**
		 * \brief Creates LLVM blocks for the \a div operation.
		 * \param left_operand Left operand
		 * \param right_operand Right operand
		 * \return Expected - value(operation result, operation precision, left operand result), error received from inner accept invocations.
		 */
		outcome::result<std::tuple<llvm::Value*, type, value_ptr>> create_div_operation(
			node* left_operand,
			node* right_operand
		);

		/**
		 * \brief Creates LLVM blocks for the \a mod operation.
		 * \param left_operand Left operand
		 * \param right_operand Right operand
		 * \return Expected - value(operation result, operation precision, left operand result), error received from inner accept invocations.
		 */
		outcome::result<std::tuple<llvm::Value*, type, value_ptr>> create_mod_operation(
			node* left_operand,
			node* right_operand
		);

		bool get_named_value(
			value_ptr& out_value,
			const std::string& variable_name
		);

		// flow control
		// codegen_visitor_flow_control.cpp
		outcome::result<value_ptr> visit_return_node(
			return_node& node,
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_if_else_node(
			if_else_node& node, 
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_while_node(
			while_node& node, 
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_for_node(
			for_node& node,
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_break_node(
			break_node& node, 
			const code_generation_context& context
		) override;

		// types
		// codegen_visitor_types.cpp
		outcome::result<value_ptr> visit_numerical_literal_node(
			numerical_literal_node& node, 
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_keyword_char_node(
			char_node& node, 
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_keyword_string_node(
			string_node& node,
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_keyword_bool_node(
			bool_node& node, 
			const code_generation_context& context
		) override;

		// utility
		value_ptr create_boolean(bool value) const;

		value_ptr create_character(char value) const;

		// operators
		// codegen_visitor_operators.cpp
		// unary
		// arithmetic
		outcome::result<value_ptr> visit_operator_post_decrement_node(
			operator_post_decrement_node& node,
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_operator_post_increment_node(
			operator_post_increment_node& node, 
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_operator_pre_decrement_node(
			operator_pre_decrement_node& node,
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_operator_pre_increment_node(
			operator_pre_increment_node& node,
			const code_generation_context& context
		) override;

		// bitwise
		outcome::result<value_ptr> visit_operator_bitwise_not_node(
			operator_bitwise_not_node& node,
			const code_generation_context& context
		) override;

		// logical
		outcome::result<value_ptr> visit_operator_not_node(
			operator_not_node& node,
			const code_generation_context& context
		) override;

		// binary
		// arithmetic
		outcome::result<value_ptr> visit_operator_addition_assignment_node(
			operator_addition_assignment_node& node,
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_operator_addition_node(
			operator_addition_node& node, 
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_operator_subtraction_assignment_node(
			operator_subtraction_assignment_node& node,
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_operator_subtraction_node(
			operator_subtraction_node& node, 
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_operator_multiplication_assignment_node(
			operator_multiplication_assignment_node& node,
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_operator_multiplication_node(
			operator_multiplication_node& node, 
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_operator_division_assignment_node(
			operator_division_assignment_node& node,
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_operator_division_node(
			operator_division_node& node, 
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_operator_modulo_assignment_node(
			operator_modulo_assignment_node& node, 
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_operator_modulo_node(
			operator_modulo_node& node,
			const code_generation_context& context
		) override;

		// bitwise
		outcome::result<value_ptr> visit_operator_bitwise_and_node(
			operator_bitwise_and_node& node,
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_operator_bitwise_or_node(
			operator_bitwise_or_node& node,
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_operator_bitwise_left_shift_node(
			operator_bitwise_left_shift_node& node,
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_operator_bitwise_right_shift_node(
			operator_bitwise_right_shift_node& node,
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_operator_bitwise_xor_node(
			operator_bitwise_xor_node& node,
			const code_generation_context& context
		) override;

		// logical
		outcome::result<value_ptr> visit_operator_logical_conjunction_node(
			operator_conjunction_node& node, 
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_operator_logical_disjunction_node(
			operator_disjunction_node& node,
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_operator_greater_than_node(
			operator_greater_than_node& node,
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_operator_greater_than_equal_to_node(
			operator_greater_than_equal_to_node& node,
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_operator_less_than_node(
			operator_less_than_node& node, 
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_operator_less_than_equal_to_node(
			operator_less_than_equal_to_node& node, 
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_operator_equals_node(
			operator_equals_node& node,
			const code_generation_context& context
		) override;

		outcome::result<value_ptr> visit_operator_not_equals_node(
			operator_not_equals_node& node, 
			const code_generation_context& context
		) override;

		// utility
		llvm::Value* cast_value(
			const value_ptr& source_value,
			type target_type, 
			const file_position& location
		);

		outcome::result<void> verify_main_entry_point();
		void initialize_global_variables() const;
		void initialize_used_external_functions() const;
	private:
		// variable utility 
		scope_ptr m_scope;
		std::unordered_map<std::string, value_ptr> m_global_named_values;
		std::vector<llvm::Constant*> m_global_ctors;
		u64 m_global_initialization_priority = 0;
		function_registry m_function_registry;
	};
}