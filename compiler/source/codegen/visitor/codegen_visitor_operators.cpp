#include "codegen_visitor.h"

#include "../abstract_syntax_tree/operators/operator_addition_node.h"
#include "../abstract_syntax_tree/operators/operator_subtraction_node.h"
#include "../abstract_syntax_tree/operators/operator_multiplication_node.h"
#include "../abstract_syntax_tree/operators/operator_division_node.h"
#include "../abstract_syntax_tree/operators/operator_modulo_node.h"

namespace channel {
	bool codegen_visitor::visit_operator_addition_node(operator_addition_node& node, value*& out_value) {
		// accept the left expression
		value* left;
		if (!node.left_expression_node->accept(*this, left)) {
			return false;
		}

		// accept the right expression
		value* right;
		if (!node.right_expression_node->accept(*this, right)) {
			return false;
		}

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(left->get_type(), right->get_type());
		llvm::Value* left_value_upcasted;
		if(!cast_value(left_value_upcasted, left, highest_precision, node.get_declaration_line_number())) {
			return false;
		}

		llvm::Value* right_value_upcasted;
		if(!cast_value(right_value_upcasted, right, highest_precision, node.get_declaration_line_number())) {
			return false;
		}

		// both types are floating point
		if (is_type_floating_point(highest_precision)) {
			out_value = new value(
				"__fadd",
				highest_precision,
				m_builder.CreateFAdd(
					left_value_upcasted,
					right_value_upcasted,
					"fadd")
			);
			return true;
		}

		// both types are unsigned
		if (is_type_unsigned(highest_precision)) {
			out_value = new value(
				"__uadd",
				highest_precision,
				m_builder.CreateAdd(
					left_value_upcasted,
					right_value_upcasted,
					"uadd",
					true)
			);
			return true;
		}

		// fallback to regular op
		out_value = new value(
			"__add",
			highest_precision,
			m_builder.CreateAdd(
				left_value_upcasted,
				right_value_upcasted,
				"add")
		);
		return true;
	}

	bool codegen_visitor::visit_operator_subtraction_node(operator_subtraction_node& node, value*& out_value) {
		// accept the left expression
		value* left;
		if (!node.left_expression_node->accept(*this, left)) {
			return false;
		}

		// accept the right expression
		value* right;
		if (!node.right_expression_node->accept(*this, right)) {
			return false;
		}

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(left->get_type(), right->get_type());
		llvm::Value* left_value_upcasted;
		if (!cast_value(left_value_upcasted, left, highest_precision, node.get_declaration_line_number())) {
			return false;
		}

		llvm::Value* right_value_upcasted;
		if (!cast_value(right_value_upcasted, right, highest_precision, node.get_declaration_line_number())) {
			return false;
		}

		// both types are floating point
		if (is_type_floating_point(highest_precision)) {
			out_value = new value(
				"__fsub",
				highest_precision,
				m_builder.CreateFSub(
					left_value_upcasted,
					right_value_upcasted,
					"fsub")
			);
			return true;
		}

		// both types are unsigned
		if (is_type_unsigned(highest_precision)) {
			out_value = new value(
				"__usub",
				highest_precision,
				m_builder.CreateSub(
					left_value_upcasted,
					right_value_upcasted,
					"usub",
					true)
			);
			return true;
		}

		// fallback to regular op
		out_value = new value(
			"__sub",
			highest_precision,
			m_builder.CreateSub(
				left_value_upcasted,
				right_value_upcasted,
				"sub")
		);
		return true;
	}

	bool codegen_visitor::visit_operator_multiplication_node(operator_multiplication_node& node, value*& out_value) {
		// accept the left expression
		value* left;
		if (!node.left_expression_node->accept(*this, left)) {
			return false;
		}

		// accept the right expression
		value* right;
		if (!node.right_expression_node->accept(*this, right)) {
			return false;
		}

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(left->get_type(), right->get_type());
		llvm::Value* left_value_upcasted;
		if (!cast_value(left_value_upcasted, left, highest_precision, node.get_declaration_line_number())) {
			return false;
		}

		llvm::Value* right_value_upcasted;
		if (!cast_value(right_value_upcasted, right, highest_precision, node.get_declaration_line_number())) {
			return false;
		}

		// both types are floating point
		if (is_type_floating_point(highest_precision)) {
			out_value = new value(
				"__fmul",
				highest_precision,
				m_builder.CreateFMul(
					left_value_upcasted,
					right_value_upcasted,
					"fmul")
			);
			return true;
		}

		// both types are unsigned
		if (is_type_unsigned(highest_precision)) {
			out_value = new value(
				"__umul",
				highest_precision,
				m_builder.CreateMul(
					left_value_upcasted,
					right_value_upcasted,
					"umul",
					true)
			);
			return true;
		}

		// fallback to regular op
		out_value = new value(
			"__mul",
			highest_precision,
			m_builder.CreateMul(
				left_value_upcasted,
				right_value_upcasted,
				"mul")
		);
		return true;
	}

	bool codegen_visitor::visit_operator_division_node(operator_division_node& node, value*& out_value) {
		// accept the left expression
		value* left;
		if (!node.left_expression_node->accept(*this, left)) {
			return false;
		}

		// accept the right expression
		value* right;
		if (!node.right_expression_node->accept(*this, right)) {
			return false;
		}

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(left->get_type(), right->get_type());
		llvm::Value* left_value_upcasted;
		if (!cast_value(left_value_upcasted, left, highest_precision, node.get_declaration_line_number())) {
			return false;
		}

		llvm::Value* right_value_upcasted;
		if (!cast_value(right_value_upcasted, right, highest_precision, node.get_declaration_line_number())) {
			return false;
		}

		// both types are floating point
		if (is_type_floating_point(highest_precision)) {
			out_value = new value(
				"__fdiv",
				highest_precision,
				m_builder.CreateFDiv(
					left_value_upcasted,
					right_value_upcasted,
					"fdiv")
			);
			return true;
		}

		// both types are unsigned
		if (is_type_unsigned(highest_precision)) {
			out_value = new value(
				"__udiv",
				highest_precision,
				m_builder.CreateUDiv(
					left_value_upcasted,
					right_value_upcasted,
					"udiv")
			);
			return true;
		}

		// fallback to regular op
		out_value = new value(
			"__div",
			highest_precision,
			m_builder.CreateSDiv(
				left_value_upcasted,
				right_value_upcasted,
				"div")
		);
		return true;
	}

	bool codegen_visitor::visit_operator_modulo_node(operator_modulo_node& node, value*& out_value) {
		// accept the left expression
		value* left;
		if (!node.left_expression_node->accept(*this, left)) {
			return false;
		}

		// accept the right expression
		value* right;
		if (!node.right_expression_node->accept(*this, right)) {
			return false;
		}

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(left->get_type(), right->get_type());
		llvm::Value* left_value_upcasted;
		if (!cast_value(left_value_upcasted, left, highest_precision, node.get_declaration_line_number())) {
			return false;
		}

		llvm::Value* right_value_upcasted;
		if (!cast_value(right_value_upcasted, right, highest_precision, node.get_declaration_line_number())) {
			return false;
		}

		// both types are floating point
		if (is_type_floating_point(highest_precision)) {
			out_value = new value(
				"__frem",
				highest_precision,
				m_builder.CreateFRem(
					left_value_upcasted,
					right_value_upcasted,
					"frem")
			);
			return true;
		}

		// both types are unsigned
		if (is_type_unsigned(highest_precision)) {
			out_value = new value(
				"__urem",
				highest_precision,
				m_builder.CreateURem(
					left_value_upcasted,
					right_value_upcasted,
					"urem")
			);
			return true;
		}

		// fallback to regular op
		out_value = new value(
			"__rem",
			highest_precision,
			m_builder.CreateSRem(
				left_value_upcasted,
				right_value_upcasted,
				"rem")
		);
		return true;
	}
}