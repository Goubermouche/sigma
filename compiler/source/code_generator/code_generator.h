#pragma once
#include "code_generator/abstract_syntax_tree/keywords/flow_control/if_else_node.h"

namespace channel {
	class translation_unit_node;

	// functions
	class function_node;
	class function_call_node;

	// variables
	class assignment_node;
	class variable_access_node;
	class local_declaration_node;
	class global_declaration_node;
	class array_allocation_node;
	class array_access_node;
	class array_assignment_node;
	class variable_node;

	// flow control
	class return_node;
	class if_else_node;
	class while_node;
	class for_node;
	class break_node;

	// types
	class numerical_literal_node;
	class char_node;
	class string_node;
	class bool_node;

	// operators
	// unary
	// arithmetic
	class operator_post_decrement;
	class operator_post_increment;
	class operator_pre_decrement;
	class operator_pre_increment;
	// binary
	// arithmetic
	class operator_addition_assignment_node;
	class operator_addition_node;
	class operator_subtraction_assignment_node;
	class operator_subtraction_node;
	class operator_multiplication_assignment_node;
	class operator_multiplication_node;
	class operator_division_assignment_node;
	class operator_division_node;
	class operator_modulo_assignment_node;
	class operator_modulo_node;
	// logical
	class operator_conjunction_node;
	class operator_disjunction_node;
	class operator_greater_than_node;
	class operator_greater_than_equal_to_node;
	class operator_less_than_node;
	class operator_less_than_equal_to_node;
	class operator_equals_node;
	class operator_not_equals_node;
}

#include "llvm_wrappers/codegen_context.h"
#include "llvm_wrappers/llvm_context.h"

namespace channel {
	class abstract_syntax_tree;

	/**
	 * \brief Codegen visitor that walks the AST and generates LLVM IR.
	 */
	class code_generator {
	public:
		code_generator();
		virtual ~code_generator() = default;

		virtual error_result generate() = 0;

		void set_abstract_syntax_tree(
			std::shared_ptr<abstract_syntax_tree> abstract_syntax_tree
		);

		std::unique_ptr<llvm_context> get_llvm_context();

		virtual acceptation_result visit_translation_unit_node(
			translation_unit_node& node,
			const codegen_context& context
		) = 0;

		// functions
		virtual acceptation_result visit_function_node(
			function_node& node,
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_function_call_node(
			function_call_node& node, 
			const codegen_context& context
		) = 0;

		// variables
		virtual acceptation_result visit_assignment_node(
			assignment_node& node, 
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_variable_access_node(
			variable_access_node& node,
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_local_declaration_node(
			local_declaration_node& node,
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_global_declaration_node(
			global_declaration_node& node, 
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_allocation_node(
			array_allocation_node& node, 
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_array_access_node(
			array_access_node& node, 
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_array_assignment_node(
			array_assignment_node& node, 
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_variable_node(
			variable_node& node,
			const codegen_context& context
		) = 0;

		// flow control
		virtual acceptation_result visit_return_node(
			return_node& node, 
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_if_else_node(
			if_else_node& node,
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_while_node(
			while_node& node,
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_for_node(
			for_node& node,
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_break_node(
			break_node& node,
			const codegen_context& context
		) = 0;

		// types
		virtual acceptation_result visit_numerical_literal_node(
			numerical_literal_node& node,
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_keyword_char_node(
			char_node& node, 
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_keyword_string_node(
			string_node& node,
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_keyword_bool_node(
			bool_node& node,
			const codegen_context& context
		) = 0;
		
		// operators
		// unary
		// arithmetic
		virtual acceptation_result visit_operator_post_decrement_node(
			operator_post_decrement& node,
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_operator_post_increment_node(
			operator_post_increment& node, 
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_operator_pre_decrement_node(
			operator_pre_decrement& node,
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_operator_pre_increment_node(
			operator_pre_increment& node, 
			const codegen_context& context
		) = 0;
		// binary
		// arithmetic
		virtual acceptation_result visit_operator_addition_assignment_node(
			operator_addition_assignment_node& node,
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_operator_addition_node(
			operator_addition_node& node,
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_operator_subtraction_assignment_node(
			operator_subtraction_assignment_node& node,
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_operator_subtraction_node(
			operator_subtraction_node& node,
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_operator_multiplication_assignment_node(
			operator_multiplication_assignment_node& node,
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_operator_multiplication_node(
			operator_multiplication_node& node,
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_operator_division_assignment_node(
			operator_division_assignment_node& node,
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_operator_division_node(
			operator_division_node& node, 
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_operator_modulo_assignment_node(
			operator_modulo_assignment_node& node,
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_operator_modulo_node(
			operator_modulo_node& node, 
			const codegen_context& context
		) = 0;

		// logical
		virtual acceptation_result visit_operator_logical_conjunction_node(
			operator_conjunction_node& node, 
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_operator_logical_disjunction_node(
			operator_disjunction_node& node,
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_operator_greater_than_node(
			operator_greater_than_node& node,
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_operator_greater_than_equal_to_node(
			operator_greater_than_equal_to_node& node, 
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_operator_less_than_node(
			operator_less_than_node& node, 
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_operator_less_than_equal_to_node(
			operator_less_than_equal_to_node& node,
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_operator_equals_node(
			operator_equals_node& node, 
			const codegen_context& context
		) = 0;

		virtual acceptation_result visit_operator_not_equals_node(
			operator_not_equals_node& node, 
			const codegen_context& context
		) = 0;
	protected:
		std::shared_ptr<abstract_syntax_tree> m_abstract_syntax_tree;
		std::unique_ptr<llvm_context> m_llvm_context;
	};
}