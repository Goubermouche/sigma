#include "codegen_visitor.h"

// arithmetic
#include "../abstract_syntax_tree/operators/arithmetic/operator_addition_node.h"
#include "../abstract_syntax_tree/operators/arithmetic/operator_subtraction_node.h"
#include "../abstract_syntax_tree/operators/arithmetic/operator_multiplication_node.h"
#include "../abstract_syntax_tree/operators/arithmetic/operator_division_node.h"
#include "../abstract_syntax_tree/operators/arithmetic/operator_modulo_node.h"
// logical
#include "../abstract_syntax_tree/operators/logical/operator_conjunction_node.h"
#include "../abstract_syntax_tree/operators/logical/operator_disjunction_node.h"
#include "../abstract_syntax_tree/operators/logical/operator_greater_than_node.h"
#include "../abstract_syntax_tree/operators/logical/operator_greater_than_equal_to_node.h"
#include "../abstract_syntax_tree/operators/logical/operator_less_than_node.h"
#include "../abstract_syntax_tree/operators/logical/operator_less_than_equal_to_node.h"
#include "../abstract_syntax_tree/operators/logical/operator_equals_node.h"
#include "../abstract_syntax_tree/operators/logical/operator_not_equals_node.h"

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
		if (highest_precision.is_floating_point()) {
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
		if (highest_precision.is_unsigned()) {
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
		if (highest_precision.is_floating_point()) {
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
		if (highest_precision.is_unsigned()) {
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
		if (highest_precision.is_floating_point()) {
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
		if (highest_precision.is_unsigned()) {
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
		if (highest_precision.is_floating_point()) {
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
		if (highest_precision.is_unsigned()) {
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
		if (highest_precision.is_floating_point()) {
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
		if (highest_precision.is_unsigned()) {
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

	bool codegen_visitor::visit_operator_logical_conjunction_node(operator_conjunction_node& node, value*& out_value) {
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

		// both expressions must be boolean
		if (left->get_type().get_base() != type::base::boolean || right->get_type().get_base() != type::base::boolean) {
			compilation_logger::emit_conjunction_operation_expects_booleans(node.get_declaration_line_number(), left->get_type(), right->get_type());
			return false;
		}

		// create a logical AND operation
		llvm::Value* and_result = m_builder.CreateAnd(left->get_value(), right->get_value(), "and");
		out_value = new value("__logical_conjunction", type(type::base::boolean, 0), and_result);
		return true;
	}

	bool codegen_visitor::visit_operator_logical_disjunction_node(operator_disjunction_node& node, value*& out_value) {
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

		// both expressions must be boolean
		if (left->get_type().get_base() != type::base::boolean || right->get_type().get_base() != type::base::boolean) {
			compilation_logger::emit_disjunction_operation_expects_booleans(node.get_declaration_line_number(), left->get_type(), right->get_type());
			return false;
		}

		// create a logical OR operation
		llvm::Value* or_result = m_builder.CreateOr(left->get_value(), right->get_value());
		out_value = new value("__logical_disjunction", type(type::base::boolean, 0), or_result);
		return true;
	}

	bool codegen_visitor::visit_operator_greater_than_node(operator_greater_than_node& node, value*& out_value) {
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

		// create a greater than operation based on the highest_precision type
		llvm::Value* greater_than_result;
		if (highest_precision.is_floating_point()) {
			greater_than_result = m_builder.CreateFCmpOGT(left_value_upcasted, right_value_upcasted);
		}
		else {
			if (highest_precision.is_unsigned()) {
				greater_than_result = m_builder.CreateICmpUGT(left_value_upcasted, right_value_upcasted);
			}
			else {
				greater_than_result = m_builder.CreateICmpSGT(left_value_upcasted, right_value_upcasted);
			}
		}

		out_value = new value("__greater_than", type(type::base::boolean, 0), greater_than_result);
		return true;
	}

	bool codegen_visitor::visit_operator_greater_than_equal_to(operator_greater_than_equal_to_node& node, value*& out_value) {
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

		// create a greater than or equal to operation based on the highest_precision type
		llvm::Value* greater_than_equal_result;
		if (highest_precision.is_floating_point()) {
			greater_than_equal_result = m_builder.CreateFCmpOGE(left_value_upcasted, right_value_upcasted);
		}
		else {
			if (highest_precision.is_unsigned()) {
				greater_than_equal_result = m_builder.CreateICmpUGE(left_value_upcasted, right_value_upcasted);
			}
			else {
				greater_than_equal_result = m_builder.CreateICmpSGE(left_value_upcasted, right_value_upcasted);
			}
		}

		out_value = new value("__greater_than_equal_to", type(type::base::boolean, 0), greater_than_equal_result);
		return true;
	}

	bool codegen_visitor::visit_operator_less_than_node(operator_less_than_node& node, value*& out_value) {
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

		// create a less than operation based on the highest_precision type
		llvm::Value* less_than_result;
		if (highest_precision.is_floating_point()) {
			less_than_result = m_builder.CreateFCmpOLT(left_value_upcasted, right_value_upcasted);
		}
		else {
			if (highest_precision.is_unsigned()) {
				less_than_result = m_builder.CreateICmpULT(left_value_upcasted, right_value_upcasted);
			}
			else {
				less_than_result = m_builder.CreateICmpSLT(left_value_upcasted, right_value_upcasted);
			}
		}

		out_value = new value("__less_than", type(type::base::boolean, 0), less_than_result);
		return true;
	}

	bool codegen_visitor::visit_operator_less_than_equal_to_node(operator_less_than_equal_to_node& node, value*& out_value) {
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

		// create a less than or equal to operation based on the highest_precision type
		llvm::Value* less_than_equal_result;
		if (highest_precision.is_floating_point()) {
			less_than_equal_result = m_builder.CreateFCmpOLE(left_value_upcasted, right_value_upcasted);
		}
		else {
			if (highest_precision.is_unsigned()) {
				less_than_equal_result = m_builder.CreateICmpULE(left_value_upcasted, right_value_upcasted);
			}
			else {
				less_than_equal_result = m_builder.CreateICmpSLE(left_value_upcasted, right_value_upcasted);
			}
		}

		out_value = new value("__less_than_equal_to", type(type::base::boolean, 0), less_than_equal_result);
		return true;
	}

	bool codegen_visitor::visit_operator_equals_node(operator_equals_node& node, value*& out_value) {
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

		// create an equals operation based on the highest_precision type
		llvm::Value* equals_result;
		if (highest_precision.is_floating_point()) {
			equals_result = m_builder.CreateFCmpOEQ(left_value_upcasted, right_value_upcasted);
		}
		else {
			equals_result = m_builder.CreateICmpEQ(left_value_upcasted, right_value_upcasted);
		}

		out_value = new value("__equals", type(type::base::boolean, 0), equals_result);
		return true;
	}

	bool codegen_visitor::visit_operator_not_equals_node(operator_not_equals_node& node, value*& out_value) {
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

		// create a not equals operation based on the highest_precision type
		llvm::Value* not_equals_result;
		if (highest_precision.is_floating_point()) {
			not_equals_result = m_builder.CreateFCmpONE(left_value_upcasted, right_value_upcasted);
		}
		else {
			not_equals_result = m_builder.CreateICmpNE(left_value_upcasted, right_value_upcasted);
		}

		out_value = new value("__not_equals", type(type::base::boolean, 0), not_equals_result);
		return true;
	}
}