#pragma once
#include "code_generator/abstract_syntax_tree/keywords/flow_control/if_else_node.h"

namespace sigma {
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
	class operator_post_decrement_node;
	class operator_post_increment_node;
	class operator_pre_decrement_node;
	class operator_pre_increment_node;
	// bitwise
	class operator_bitwise_not_node;
	class operator_address_of_node;
	class operator_dereference_node;
	// logical
	class operator_not_node;
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
	// bitwise
	class operator_bitwise_and_node;
	class operator_bitwise_or_node;
	class operator_bitwise_left_shift_node;
	class operator_bitwise_right_shift_node;
	class operator_bitwise_xor_node;
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

#include "llvm_wrappers/code_generation_context.h"
#include "code_generator/code_generator_context.h"

namespace sigma {
	class abstract_syntax_tree;

	/**
	 * \brief Codegen visitor that walks the AST and generates LLVM IR.
	 */
	class code_generator_template {
	public:
		virtual ~code_generator_template() = default;

		// functions
		virtual outcome::result<value_ptr> visit_function_node(
			function_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_function_call_node(
			function_call_node& node, 
			const code_generation_context& context
		) = 0;

		// variables
		virtual outcome::result<value_ptr> visit_assignment_node(
			assignment_node& node, 
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_variable_access_node(
			variable_access_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_local_declaration_node(
			local_declaration_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_global_declaration_node(
			global_declaration_node& node, 
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_allocation_node(
			array_allocation_node& node, 
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_array_access_node(
			array_access_node& node, 
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_array_assignment_node(
			array_assignment_node& node, 
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_variable_node(
			variable_node& node,
			const code_generation_context& context
		) = 0;

		// flow control
		virtual outcome::result<value_ptr> visit_return_node(
			return_node& node, 
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_if_else_node(
			if_else_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_while_node(
			while_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_for_node(
			for_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_break_node(
			break_node& node,
			const code_generation_context& context
		) = 0;

		// types
		virtual outcome::result<value_ptr> visit_numerical_literal_node(
			numerical_literal_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_keyword_char_node(
			char_node& node, 
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_keyword_string_node(
			string_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_keyword_bool_node(
			bool_node& node,
			const code_generation_context& context
		) = 0;
		
		// operators
		// unary
		// arithmetic
		virtual outcome::result<value_ptr> visit_operator_post_decrement_node(
			operator_post_decrement_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_post_increment_node(
			operator_post_increment_node& node, 
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_pre_decrement_node(
			operator_pre_decrement_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_pre_increment_node(
			operator_pre_increment_node& node, 
			const code_generation_context& context
		) = 0;

		// bitwise
		virtual outcome::result<value_ptr> visit_operator_bitwise_not_node(
			operator_bitwise_not_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_address_of_node(
			operator_address_of_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_dereference_node(
			operator_dereference_node& node,
			const code_generation_context& context
		) = 0;

		// logical
		virtual outcome::result<value_ptr> visit_operator_not_node(
			operator_not_node& node,
			const code_generation_context& context
		) = 0;
		
		// binary
		// arithmetic
		virtual outcome::result<value_ptr> visit_operator_addition_assignment_node(
			operator_addition_assignment_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_addition_node(
			operator_addition_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_subtraction_assignment_node(
			operator_subtraction_assignment_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_subtraction_node(
			operator_subtraction_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_multiplication_assignment_node(
			operator_multiplication_assignment_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_multiplication_node(
			operator_multiplication_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_division_assignment_node(
			operator_division_assignment_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_division_node(
			operator_division_node& node, 
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_modulo_assignment_node(
			operator_modulo_assignment_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_modulo_node(
			operator_modulo_node& node, 
			const code_generation_context& context
		) = 0;

		// bitwise
		virtual outcome::result<value_ptr> visit_operator_bitwise_and_node(
			operator_bitwise_and_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_bitwise_or_node(
			operator_bitwise_or_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_bitwise_left_shift_node(
			operator_bitwise_left_shift_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_bitwise_right_shift_node(
			operator_bitwise_right_shift_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_bitwise_xor_node(
			operator_bitwise_xor_node& node,
			const code_generation_context& context
		) = 0;

		// logical
		virtual outcome::result<value_ptr> visit_operator_logical_conjunction_node(
			operator_conjunction_node& node, 
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_logical_disjunction_node(
			operator_disjunction_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_greater_than_node(
			operator_greater_than_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_greater_than_equal_to_node(
			operator_greater_than_equal_to_node& node, 
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_less_than_node(
			operator_less_than_node& node, 
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_less_than_equal_to_node(
			operator_less_than_equal_to_node& node,
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_equals_node(
			operator_equals_node& node, 
			const code_generation_context& context
		) = 0;

		virtual outcome::result<value_ptr> visit_operator_not_equals_node(
			operator_not_equals_node& node, 
			const code_generation_context& context
		) = 0;
	};
}