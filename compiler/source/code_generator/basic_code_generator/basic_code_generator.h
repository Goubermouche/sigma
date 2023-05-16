#pragma once

#include "parser/parser.h"
#include "code_generator/code_generator.h"
#include "llvm_wrappers/scope.h"
#include "llvm_wrappers/codegen_context.h"
#include "llvm_wrappers/functions/function_registry.h"

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
	class basic_code_generator : public code_generator {
	public:
		basic_code_generator();

		error_result generate() override;
		std::shared_ptr<llvm::Module> get_module();

		void print_intermediate_representation() const;
		error_result verify_intermediate_representation() const;
	private:
		acceptation_result visit_translation_unit_node(
			translation_unit_node& node, 
			const codegen_context& context
		) override;

		// functions
		// codegen_visitor_functions.cpp
		acceptation_result visit_function_node(
			function_node& node, 
			const codegen_context& context
		) override;

		acceptation_result visit_function_call_node(
			function_call_node& node, 
			const codegen_context& context
		) override;

		// variables
		// codegen_visitor_variables.cpp
		acceptation_result visit_assignment_node(
			assignment_node& node,
			const codegen_context& context
		) override;

		acceptation_result visit_variable_access_node(
			variable_access_node& node, 
			const codegen_context& context
		) override;

		acceptation_result visit_local_declaration_node(
			local_declaration_node& node,
			const codegen_context& context
		) override;

		acceptation_result visit_global_declaration_node(
			global_declaration_node& node,
			const codegen_context& context
		) override;

		acceptation_result visit_allocation_node(
			array_allocation_node& node, 
			const codegen_context& context
		) override;

		acceptation_result visit_array_access_node(
			array_access_node& node,
			const codegen_context& context
		) override;

		acceptation_result visit_array_assignment_node(
			array_assignment_node& node, 
			const codegen_context& context
		) override;

		acceptation_result visit_variable_node(
			variable_node& node, 
			const codegen_context& context
		) override;

		// utility
		acceptation_result get_declaration_value(
			const declaration_node& node,
			const codegen_context& context
		);

		/**
		 * \brief Creates LLVM blocks for the \a add operation.
		 * \param left_operand Left operand
		 * \param right_operand Right operand
		 * \return Expected - value(operation result, operation precision, left operand result), error received from inner accept invocations.
		 */
		std::expected<std::tuple<llvm::Value*, type, acceptation_result>, error_message> create_add_operation(
			node* left_operand,
			node* right_operand
		);

		/**
		 * \brief Creates LLVM blocks for the \a sub operation.
		 * \param left_operand Left operand
		 * \param right_operand Right operand
		 * \return Expected - value(operation result, operation precision, left operand result), error received from inner accept invocations.
		 */
		std::expected<std::tuple<llvm::Value*, type, acceptation_result>, error_message> create_sub_operation(
			node* left_operand,
			node* right_operand
		);

		/**
		 * \brief Creates LLVM blocks for the \a mul operation.
		 * \param left_operand Left operand
		 * \param right_operand Right operand
		 * \return Expected - value(operation result, operation precision, left operand result), error received from inner accept invocations.
		 */
		std::expected<std::tuple<llvm::Value*, type, acceptation_result>, error_message> create_mul_operation(
			node* left_operand,
			node* right_operand
		);

		/**
		 * \brief Creates LLVM blocks for the \a div operation.
		 * \param left_operand Left operand
		 * \param right_operand Right operand
		 * \return Expected - value(operation result, operation precision, left operand result), error received from inner accept invocations.
		 */
		std::expected<std::tuple<llvm::Value*, type, acceptation_result>, error_message> create_div_operation(
			node* left_operand,
			node* right_operand
		);

		/**
		 * \brief Creates LLVM blocks for the \a mod operation.
		 * \param left_operand Left operand
		 * \param right_operand Right operand
		 * \return Expected - value(operation result, operation precision, left operand result), error received from inner accept invocations.
		 */
		std::expected<std::tuple<llvm::Value*, type, acceptation_result>, error_message> create_mod_operation(
			node* left_operand,
			node* right_operand
		);

		bool get_named_value(
			value_ptr& out_value,
			const std::string& variable_name
		);

		// flow control
		// codegen_visitor_flow_control.cpp
		acceptation_result visit_return_node(
			return_node& node,
			const codegen_context& context
		) override;

		acceptation_result visit_if_else_node(
			if_else_node& node, 
			const codegen_context& context
		) override;

		acceptation_result visit_while_node(
			while_node& node, 
			const codegen_context& context
		) override;

		acceptation_result visit_for_node(
			for_node& node,
			const codegen_context& context
		) override;

		acceptation_result visit_break_node(
			break_node& node, 
			const codegen_context& context
		) override;

		// types
		// codegen_visitor_types.cpp
		acceptation_result visit_numerical_literal_node(
			numerical_literal_node& node, 
			const codegen_context& context
		) override;

		acceptation_result visit_keyword_char_node(
			char_node& node, 
			const codegen_context& context
		) override;

		acceptation_result visit_keyword_string_node(
			string_node& node,
			const codegen_context& context
		) override;

		acceptation_result visit_keyword_bool_node(
			bool_node& node, 
			const codegen_context& context
		) override;

		// utility
		value_ptr create_boolean(bool value);

		value_ptr create_character(char value);

		// operators
		// codegen_visitor_operators.cpp
		// unary
		// arithmetic
		acceptation_result visit_operator_post_decrement_node(
			operator_post_decrement& node,
			const codegen_context& context
		) override;

		acceptation_result visit_operator_post_increment_node(
			operator_post_increment& node, 
			const codegen_context& context
		) override;

		acceptation_result visit_operator_pre_decrement_node(
			operator_pre_decrement& node,
			const codegen_context& context
		) override;

		acceptation_result visit_operator_pre_increment_node(
			operator_pre_increment& node,
			const codegen_context& context
		) override;

		// binary
		// arithmetic
		acceptation_result visit_operator_addition_assignment_node(
			operator_addition_assignment_node& node,
			const codegen_context& context
		) override;

		acceptation_result visit_operator_addition_node(
			operator_addition_node& node, 
			const codegen_context& context
		) override;

		acceptation_result visit_operator_subtraction_assignment_node(
			operator_subtraction_assignment_node& node,
			const codegen_context& context
		) override;

		acceptation_result visit_operator_subtraction_node(
			operator_subtraction_node& node, 
			const codegen_context& context
		) override;

		acceptation_result visit_operator_multiplication_assignment_node(
			operator_multiplication_assignment_node& node,
			const codegen_context& context
		) override;

		acceptation_result visit_operator_multiplication_node(
			operator_multiplication_node& node, 
			const codegen_context& context
		) override;

		acceptation_result visit_operator_division_assignment_node(
			operator_division_assignment_node& node,
			const codegen_context& context
		) override;

		acceptation_result visit_operator_division_node(
			operator_division_node& node, 
			const codegen_context& context
		) override;

		acceptation_result visit_operator_modulo_assignment_node(
			operator_modulo_assignment_node& node, 
			const codegen_context& context
		) override;

		acceptation_result visit_operator_modulo_node(
			operator_modulo_node& node,
			const codegen_context& context
		) override;

		// logical
		acceptation_result visit_operator_logical_conjunction_node(
			operator_conjunction_node& node, 
			const codegen_context& context
		) override;

		acceptation_result visit_operator_logical_disjunction_node(
			operator_disjunction_node& node,
			const codegen_context& context
		) override;

		acceptation_result visit_operator_greater_than_node(
			operator_greater_than_node& node,
			const codegen_context& context
		) override;

		acceptation_result visit_operator_greater_than_equal_to_node(
			operator_greater_than_equal_to_node& node,
			const codegen_context& context
		) override;

		acceptation_result visit_operator_less_than_node(
			operator_less_than_node& node, 
			const codegen_context& context
		) override;

		acceptation_result visit_operator_less_than_equal_to_node(
			operator_less_than_equal_to_node& node, 
			const codegen_context& context
		) override;

		acceptation_result visit_operator_equals_node(
			operator_equals_node& node,
			const codegen_context& context
		) override;

		acceptation_result visit_operator_not_equals_node(
			operator_not_equals_node& node, 
			const codegen_context& context
		) override;

		// utility
		llvm::Value* cast_value(
			const value_ptr& source_value,
			type target_type, 
			const token_position& position
		);

		error_result verify_main_entry_point() const;
		void initialize_global_variables();
	private:
		scope_ptr m_scope;
		std::unordered_map<std::string, value_ptr> m_global_named_values;
		std::vector<llvm::Constant*> m_global_ctors;
		u64 m_global_initialization_priority = 0;
		function_registry m_function_registry;
	};
}