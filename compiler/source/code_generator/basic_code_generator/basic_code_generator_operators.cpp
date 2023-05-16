#include "basic_code_generator.h"

// unary
// arithmetic
#include "code_generator/abstract_syntax_tree/operators/unary/arithmetic/operator_post_decrement.h"
#include "code_generator/abstract_syntax_tree/operators/unary/arithmetic/operator_post_increment.h"
#include "code_generator/abstract_syntax_tree/operators/unary/arithmetic/operator_pre_decrement.h"
#include "code_generator/abstract_syntax_tree/operators/unary/arithmetic/operator_pre_increment.h"
// binary
// arithmetic
#include "code_generator/abstract_syntax_tree/operators/binary/arithmetic/operator_addition_assignment_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/arithmetic/operator_addition_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/arithmetic/operator_subtraction_assignment_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/arithmetic/operator_subtraction_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/arithmetic/operator_multiplication_assignment_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/arithmetic/operator_multiplication_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/arithmetic/operator_division_assignment_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/arithmetic/operator_division_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/arithmetic/operator_modulo_assignment_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/arithmetic/operator_modulo_node.h"
// logical
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_conjunction_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_disjunction_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_greater_than_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_greater_than_equal_to_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_less_than_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_less_than_equal_to_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_equals_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_not_equals_node.h"

namespace channel {
	// unary
	// arithmetic
	acceptation_result basic_code_generator::visit_operator_post_decrement_node(
		operator_post_decrement& node,
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		acceptation_result expression_result = node.get_expression_node()->accept(
			*this, 
			{}
		);

		if (!expression_result.has_value()) {
			return expression_result;
		}

		// check if the expression is an integer or a floating-point value
		if (!expression_result.value()->get_type().is_numerical()) {
			return std::unexpected(
				error::emit<4007>(
					node.get_declared_position(), 
					expression_result.value()->get_type()
				)
			); // return on failure
		}

		llvm::Value* decrement_result;
		if (expression_result.value()->get_type().is_floating_point()) {
			decrement_result = m_builder.CreateFSub(
				expression_result.value()->get_value(),
				llvm::ConstantFP::get(
					expression_result.value()->get_type().get_llvm_type(
						m_context
					), 
					1.0
				)
			);
		}
		else {
			decrement_result = m_builder.CreateSub(
				expression_result.value()->get_value(),
				llvm::ConstantInt::get(
					expression_result.value()->get_type().get_llvm_type(m_context),
					1
				)
			);
		}

		// assert that the pointer is not nullptr
		ASSERT(expression_result.value()->get_pointer() != nullptr, "pointer is nullptr");

		m_builder.CreateStore(
			decrement_result,
			expression_result.value()->get_pointer()
		);

		return expression_result;
	}

	acceptation_result basic_code_generator::visit_operator_post_increment_node(
		operator_post_increment& node,
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		acceptation_result expression_result = node.get_expression_node()->accept(
			*this,
			{}
		);

		if (!expression_result.has_value()) {
			return expression_result;
		}

		// check if the expression is an integer or a floating-point value
		if (!expression_result.value()->get_type().is_numerical()) {
			return std::unexpected(
				error::emit<4007>(
					node.get_declared_position(),
					expression_result.value()->get_type()
				)
			); // return on failure
		}

		llvm::Value* decrement_result;
		if (expression_result.value()->get_type().is_floating_point()) {
			decrement_result = m_builder.CreateFAdd(
				expression_result.value()->get_value(),
				llvm::ConstantFP::get(
					expression_result.value()->get_type().get_llvm_type(m_context),
					1.0
				)
			);
		}
		else {
			decrement_result = m_builder.CreateAdd(
				expression_result.value()->get_value(), 
				llvm::ConstantInt::get(
					expression_result.value()->get_type().get_llvm_type(m_context),
					1
				)
			);
		}

		// assert that the pointer is not nullptr
		ASSERT(expression_result.value()->get_pointer() != nullptr, "pointer is nullptr");

		m_builder.CreateStore(
			decrement_result,
			expression_result.value()->get_pointer()
		);

		return expression_result;
	}

	acceptation_result basic_code_generator::visit_operator_pre_decrement_node(
		operator_pre_decrement& node, 
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		// accept the expression
		acceptation_result expression_result = node.get_expression_node()->accept(
			*this,
			{}
		);

		if (!expression_result.has_value()) {
			return expression_result;
		}

		// check if the expression is an integer or a floating-point value
		if (!expression_result.value()->get_type().is_numerical()) {
			return std::unexpected(
				error::emit<4007>(
					node.get_declared_position(), 
					expression_result.value()->get_type()
				)
			); // return on failure
		}

		// increment the expression
		llvm::Value* increment_result;
		if (expression_result.value()->get_type().is_floating_point()) {
			increment_result = m_builder.CreateFSub(
				expression_result.value()->get_value(),
				llvm::ConstantFP::get(
					expression_result.value()->get_type().get_llvm_type(m_context), 
					1.0
				)
			);
		}
		else {
			increment_result = m_builder.CreateSub(
				expression_result.value()->get_value(),
				llvm::ConstantInt::get(
					expression_result.value()->get_type().get_llvm_type(m_context),
					1
				)
			);
		}

		// assert that the pointer is not nullptr
		ASSERT(expression_result.value()->get_pointer() != nullptr, "pointer is nullptr");

		// store the decremented value back to memory
		m_builder.CreateStore(
			increment_result,
			expression_result.value()->get_pointer()
		);

		return std::make_shared<value>(
			"__pre_increment", 
			expression_result.value()->get_type(),
			increment_result
		);
	}

	acceptation_result basic_code_generator::visit_operator_pre_increment_node(
		operator_pre_increment& node, 
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		// accept the expression
		acceptation_result expression_result = node.get_expression_node()->accept(
			*this,
			{}
		);

		if (!expression_result.has_value()) {
			return expression_result;
		}

		// check if the expression is an integer or a floating-point value
		if (!expression_result.value()->get_type().is_numerical()) {
			return std::unexpected(
				error::emit<4007>(
					node.get_declared_position(), 
					expression_result.value()->get_type()
				)
			); // return on failure
		}

		// increment the expression
		llvm::Value* increment_result;
		if (expression_result.value()->get_type().is_floating_point()) {
			increment_result = m_builder.CreateFAdd(
				expression_result.value()->get_value(), 
				llvm::ConstantFP::get(
					expression_result.value()->get_type().get_llvm_type(m_context),
					1.0
				)
			);
		}
		else {
			increment_result = m_builder.CreateAdd(
				expression_result.value()->get_value(),
				llvm::ConstantInt::get(
					expression_result.value()->get_type().get_llvm_type(m_context),
					1
				)
			);
		}

		// assert that the pointer is not nullptr
		ASSERT(expression_result.value()->get_pointer() != nullptr, "pointer is nullptr");

		// store the incremented value back to memory
		m_builder.CreateStore(
			increment_result,
			expression_result.value()->get_pointer()
		);

		return std::make_shared<value>(
			"__pre_increment", 
			expression_result.value()->get_type(),
			increment_result
		);
	}

	// binary
	// arithmetic
	acceptation_result basic_code_generator::visit_operator_addition_assignment_node(
		operator_addition_assignment_node& node,
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		auto operation_result = create_add_operation(
			node.get_left_expression_node(),
			node.get_right_expression_node()
		);

		if(!operation_result.has_value()) {
			return std::unexpected(
				operation_result.error()
			); // return on failure
		}

		const auto& [
			result_value,
			highest_precision, 
			left_operand_result
		] = operation_result.value();

		// create the assignment value
		auto assignment_value = std::make_shared<value>(
			"__add_assign",
			highest_precision,
			result_value
		);

		// store the result of the addition operation back into the variable
		m_builder.CreateStore(
			assignment_value->get_value(), 
			left_operand_result.value()->get_pointer()
		);

		return assignment_value;
	}

	acceptation_result basic_code_generator::visit_operator_addition_node(
		operator_addition_node& node,
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		auto operation_result = create_add_operation(
			node.get_left_expression_node(), 
			node.get_right_expression_node()
		);

		if (!operation_result.has_value()) {
			return std::unexpected(
				operation_result.error()
			); // return on failure
		}

		const auto& [
			result_value, 
			highest_precision, 
			left_operand_result
		] = operation_result.value();

		return std::make_shared<value>(
			"__add",
			highest_precision,
			result_value
		);
	}

	acceptation_result basic_code_generator::visit_operator_subtraction_assignment_node(
		operator_subtraction_assignment_node& node,
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		auto operation_result = create_sub_operation(
			node.get_left_expression_node(), 
			node.get_right_expression_node()
		);

		if (!operation_result.has_value()) {
			return std::unexpected(
				operation_result.error()
			); // return on failure
		}

		const auto& [
			result_value, 
			highest_precision, 
			left_operand_result
		] = operation_result.value();

		// create the assignment value
		auto assignment_value = std::make_shared<value>(
			"__sub_assign",
			highest_precision,
			result_value
		);

		// store the result of the subtraction operation back into the variable
		m_builder.CreateStore(
			assignment_value->get_value(),
			left_operand_result.value()->get_pointer()
		);

		return assignment_value;
	}

	acceptation_result basic_code_generator::visit_operator_subtraction_node(
		operator_subtraction_node& node,
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		auto operation_result = create_sub_operation(
			node.get_left_expression_node(),
			node.get_right_expression_node()
		);

		if (!operation_result.has_value()) {
			return std::unexpected(
				operation_result.error()
			); // return on failure
		}

		const auto& [
			result_value,
			highest_precision, 
			left_operand_result
		] = operation_result.value();

		return std::make_shared<value>(
			"__sub",
			highest_precision,
			result_value
		);
	}

	acceptation_result basic_code_generator::visit_operator_multiplication_assignment_node(
		operator_multiplication_assignment_node& node, 
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		auto operation_result = create_mul_operation(
			node.get_left_expression_node(), 
			node.get_right_expression_node()
		);

		if (!operation_result.has_value()) {
			return std::unexpected(
				operation_result.error()
			); // return on failure
		}

		const auto& [
			result_value,
			highest_precision,
			left_operand_result
		] = operation_result.value();

		// create the assignment value
		auto assignment_value = std::make_shared<value>(
			"__sub_assign",
			highest_precision,
			result_value
		);

		// store the result of the multiplication operation back into the variable
		m_builder.CreateStore(
			assignment_value->get_value(),
			left_operand_result.value()->get_pointer()
		);

		return assignment_value;
	}

	acceptation_result basic_code_generator::visit_operator_multiplication_node(
		operator_multiplication_node& node,
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		auto operation_result = create_mul_operation(
			node.get_left_expression_node(), 
			node.get_right_expression_node()
		);

		if (!operation_result.has_value()) {
			return std::unexpected(
				operation_result.error()
			); // return on failure
		}

		const auto& [
			result_value, 
			highest_precision,
			left_operand_result
		] = operation_result.value();

		return std::make_shared<value>(
			"__mul",
			highest_precision,
			result_value
		);
	}

	acceptation_result basic_code_generator::visit_operator_division_assignment_node(
		operator_division_assignment_node& node, 
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		auto operation_result = create_div_operation(
			node.get_left_expression_node(),
			node.get_right_expression_node()
		);

		if (!operation_result.has_value()) {
			return std::unexpected(
				operation_result.error()
			); // return on failure
		}

		const auto& [
			result_value, 
			highest_precision,
			left_operand_result
		] = operation_result.value();

		// create the assignment value
		auto assignment_value = std::make_shared<value>(
			"__div_assign",
			highest_precision,
			result_value
		);

		// store the result of the division operation back into the variable
		m_builder.CreateStore(
			assignment_value->get_value(),
			left_operand_result.value()->get_pointer()
		);

		return assignment_value;
	}

	acceptation_result basic_code_generator::visit_operator_division_node(
		operator_division_node& node, 
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		auto operation_result = create_div_operation(
			node.get_left_expression_node(),
			node.get_right_expression_node()
		);

		if (!operation_result.has_value()) {
			return std::unexpected(
				operation_result.error()
			); // return on failure
		}

		const auto& [
			result_value,
			highest_precision, 
			left_operand_result
		] = operation_result.value();

		return std::make_shared<value>(
			"__div",
			highest_precision,
			result_value
		);
	}

	acceptation_result basic_code_generator::visit_operator_modulo_assignment_node(
		operator_modulo_assignment_node& node,
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		auto operation_result = create_mod_operation(
			node.get_left_expression_node(),
			node.get_right_expression_node()
		);

		if (!operation_result.has_value()) {
			return std::unexpected(
				operation_result.error()
			); // return on failure
		}

		const auto& [
			result_value, 
			highest_precision,
			left_operand_result
		] = operation_result.value();

		// create the assignment value
		auto assignment_value = std::make_shared<value>(
			"__mod_assign",
			highest_precision,
			result_value
		);

		// store the result of the modulo operation back into the variable
		m_builder.CreateStore(
			assignment_value->get_value(),
			left_operand_result.value()->get_pointer()
		);

		return assignment_value;
	}

	acceptation_result basic_code_generator::visit_operator_modulo_node(
		operator_modulo_node& node,
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		auto operation_result = create_mod_operation(
			node.get_left_expression_node(),
			node.get_right_expression_node()
		);

		if (!operation_result.has_value()) {
			return std::unexpected(
				operation_result.error()
			); // return on failure
		}

		const auto& [
			result_value,
			highest_precision,
			left_operand_result
		] = operation_result.value();

		return std::make_shared<value>(
			"__mod",
			highest_precision,
			result_value
		);
	}

	// logical
	acceptation_result basic_code_generator::visit_operator_logical_conjunction_node(
		operator_conjunction_node& node,
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		// accept the left operand
		acceptation_result left_operand_result = node.get_left_expression_node()->accept(
			*this,
			{}
		);

		if (!left_operand_result.has_value()) {
			return left_operand_result; // return on failure
		}

		// accept the right operand
		acceptation_result right_operand_result = node.get_right_expression_node()->accept(
			*this,
			{}
		);

		if (!right_operand_result.has_value()) {
			return right_operand_result; // return on failure
		}

		// both expressions must be boolean
		if (left_operand_result.value()->get_type().get_base() != type::base::boolean || 
			right_operand_result.value()->get_type().get_base() != type::base::boolean) {
			return std::unexpected(
				error::emit<4008>(
					node.get_declared_position(),
					left_operand_result.value()->get_type(),
					right_operand_result.value()->get_type()
				)
			); // return on failure
		}

		// create a logical AND operation
		llvm::Value* and_result = m_builder.CreateAnd(
			left_operand_result.value()->get_value(),
			right_operand_result.value()->get_value(),
			"and"
		);

		return std::make_shared<value>(
			"__logical_conjunction", 
			type(type::base::boolean, 0),
			and_result
		);
	}

	acceptation_result basic_code_generator::visit_operator_logical_disjunction_node(
		operator_disjunction_node& node, 
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		// accept the left operand
		acceptation_result left_operand_result = node.get_left_expression_node()->accept(
			*this, 
			{}
		);

		if (!left_operand_result.has_value()) {
			return left_operand_result; // return on failure
		}

		// accept the right operand
		acceptation_result right_operand_result = node.get_right_expression_node()->accept(
			*this,
			{}
		);

		if (!right_operand_result.has_value()) {
			return right_operand_result;
		}

		// both expressions must be boolean
		if (left_operand_result.value()->get_type().get_base() != type::base::boolean ||
			right_operand_result.value()->get_type().get_base() != type::base::boolean) {
			return std::unexpected(
				error::emit<4009>(node.get_declared_position(),
					left_operand_result.value()->get_type(), 
					right_operand_result.value()->get_type()
				)
			); // return on failure
		}

		// create a logical OR operation
		llvm::Value* or_result = m_builder.CreateOr(
			left_operand_result.value()->get_value(),
			right_operand_result.value()->get_value()
		);

		return std::make_shared<value>(
			"__logical_disjunction",
			type(type::base::boolean, 0),
			or_result
		);
	}

	acceptation_result basic_code_generator::visit_operator_greater_than_node(
		operator_greater_than_node& node, 
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		// accept the left operand
		acceptation_result left_operand_result = node.get_left_expression_node()->accept(
			*this,
			{}
		);

		if (!left_operand_result.has_value()) {
			return left_operand_result; // return on failure
		}

		// accept the right operand
		acceptation_result right_operand_result = node.get_right_expression_node()->accept(
			*this,
			{}
		);

		if (!right_operand_result.has_value()) {
			return right_operand_result; // return on failure
		}

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(
			left_operand_result.value()->get_type(),
			right_operand_result.value()->get_type()
		);

		llvm::Value* left_value_upcasted = cast_value(
			left_operand_result.value(),
			highest_precision,
			node.get_declared_position()
		);

		llvm::Value* right_value_upcasted = cast_value(
			right_operand_result.value(), 
			highest_precision, 
			node.get_declared_position()
		);

		// create a greater than operation based on the highest_precision type
		llvm::Value* greater_than_result;
		if (highest_precision.is_floating_point()) {
			greater_than_result = m_builder.CreateFCmpOGT(
				left_value_upcasted,
				right_value_upcasted
			);
		}
		else {
			if (highest_precision.is_unsigned()) {
				greater_than_result = m_builder.CreateICmpUGT(
					left_value_upcasted, 
					right_value_upcasted
				);
			}
			else {
				greater_than_result = m_builder.CreateICmpSGT(
					left_value_upcasted,
					right_value_upcasted
				);
			}
		}

		return std::make_shared<value>(
			"__greater_than",
			type(type::base::boolean, 0),
			greater_than_result
		);
	}

	acceptation_result basic_code_generator::visit_operator_greater_than_equal_to_node(
		operator_greater_than_equal_to_node& node, 
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		// accept the left operand
		acceptation_result left_operand_result = node.get_left_expression_node()->accept(
			*this,
			{}
		);

		if (!left_operand_result.has_value()) {
			return left_operand_result; // return on failure
		}

		// accept the right operand
		acceptation_result right_operand_result = node.get_right_expression_node()->accept(
			*this,
			{}
		);

		if (!right_operand_result.has_value()) {
			return right_operand_result; // return on failure
		}

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(
			left_operand_result.value()->get_type(),
			right_operand_result.value()->get_type()
		);

		llvm::Value* left_value_upcasted = cast_value(
			left_operand_result.value(),
			highest_precision,
			node.get_declared_position()
		);

		llvm::Value* right_value_upcasted = cast_value(
			right_operand_result.value(), 
			highest_precision,
			node.get_declared_position()
		);

		// create a greater than or equal to operation based on the highest_precision type
		llvm::Value* greater_than_equal_result;
		if (highest_precision.is_floating_point()) {
			greater_than_equal_result = m_builder.CreateFCmpOGE(
				left_value_upcasted, 
				right_value_upcasted
			);
		}
		else {
			if (highest_precision.is_unsigned()) {
				greater_than_equal_result = m_builder.CreateICmpUGE(
					left_value_upcasted, 
					right_value_upcasted
				);
			}
			else {
				greater_than_equal_result = m_builder.CreateICmpSGE(
					left_value_upcasted, 
					right_value_upcasted
				);
			}
		}

		return std::make_shared<value>(
			"__greater_than_equal_to",
			type(type::base::boolean, 0),
			greater_than_equal_result
		);
	}

	acceptation_result basic_code_generator::visit_operator_less_than_node(
		operator_less_than_node& node, 
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		// accept the left operand
		acceptation_result left_operand_result = node.get_left_expression_node()->accept(
			*this, 
			{}
		);

		if (!left_operand_result.has_value()) {
			return left_operand_result; // return on failure
		}

		// accept the right operand
		acceptation_result right_operand_result = node.get_right_expression_node()->accept(
			*this,
			{}
		);

		if (!right_operand_result.has_value()) {
			return right_operand_result; // return on failure
		}

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(
			left_operand_result.value()->get_type(),
			right_operand_result.value()->get_type()
		);

		llvm::Value* left_value_upcasted = cast_value(
			left_operand_result.value(),
			highest_precision,
			node.get_declared_position()
		);

		llvm::Value* right_value_upcasted = cast_value(
			right_operand_result.value(), 
			highest_precision,
			node.get_declared_position()
		);

		// create a less than operation based on the highest_precision type
		llvm::Value* less_than_result;
		if (highest_precision.is_floating_point()) {
			less_than_result = m_builder.CreateFCmpOLT(
				left_value_upcasted, 
				right_value_upcasted
			);
		}
		else {
			if (highest_precision.is_unsigned()) {
				less_than_result = m_builder.CreateICmpULT(
					left_value_upcasted,
					right_value_upcasted
				);
			}
			else {
				less_than_result = m_builder.CreateICmpSLT(
					left_value_upcasted,
					right_value_upcasted
				);
			}
		}

		return std::make_shared<value>(
			"__less_than",
			type(type::base::boolean, 0),
			less_than_result
		);
	}

	acceptation_result basic_code_generator::visit_operator_less_than_equal_to_node(
		operator_less_than_equal_to_node& node,
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		// accept the left operand
		acceptation_result left_operand_result = node.get_left_expression_node()->accept(
			*this,
			{}
		);

		if (!left_operand_result.has_value()) {
			return left_operand_result; // return on failure
		}

		// accept the right operand
		acceptation_result right_operand_result = node.get_right_expression_node()->accept(
			*this,
			{}
		);

		if (!right_operand_result.has_value()) {
			return right_operand_result; // return on failure
		}

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(
			left_operand_result.value()->get_type(),
			right_operand_result.value()->get_type()
		);

		llvm::Value* left_value_upcasted = cast_value(
			left_operand_result.value(),
			highest_precision,
			node.get_declared_position()
		);

		llvm::Value* right_value_upcasted = cast_value(
			right_operand_result.value(), 
			highest_precision, 
			node.get_declared_position()
		);

		// create a less than or equal to operation based on the highest_precision type
		llvm::Value* less_than_equal_result;
		if (highest_precision.is_floating_point()) {
			less_than_equal_result = m_builder.CreateFCmpOLE(
				left_value_upcasted,
				right_value_upcasted
			);
		}
		else {
			if (highest_precision.is_unsigned()) {
				less_than_equal_result = m_builder.CreateICmpULE(
					left_value_upcasted,
					right_value_upcasted
				);
			}
			else {
				less_than_equal_result = m_builder.CreateICmpSLE(
					left_value_upcasted, 
					right_value_upcasted
				);
			}
		}

		return std::make_shared<value>(
			"__less_than_equal_to",
			type(type::base::boolean, 0), 
			less_than_equal_result
		);
	}

	acceptation_result basic_code_generator::visit_operator_equals_node(
		operator_equals_node& node, 
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		// accept the left operand
		acceptation_result left_operand_result = node.get_left_expression_node()->accept(
			*this, 
			{}
		);

		if (!left_operand_result.has_value()) {
			return left_operand_result; // return on failure
		}

		// accept the right operand
		acceptation_result right_operand_result = node.get_right_expression_node()->accept(
			*this,
			{}
		);

		if (!right_operand_result.has_value()) {
			return right_operand_result; // return on failure
		}

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(
			left_operand_result.value()->get_type(),
			right_operand_result.value()->get_type()
		);

		llvm::Value* left_value_upcasted = cast_value(
			left_operand_result.value(),
			highest_precision,
			node.get_declared_position()
		);

		llvm::Value* right_value_upcasted = cast_value(
			right_operand_result.value(),
			highest_precision,
			node.get_declared_position()
		);

		// create an equals operation based on the highest_precision type
		llvm::Value* equals_result;
		if (highest_precision.is_floating_point()) {
			equals_result = m_builder.CreateFCmpOEQ(
				left_value_upcasted, 
				right_value_upcasted
			);
		}
		else {
			equals_result = m_builder.CreateICmpEQ(
				left_value_upcasted, 
				right_value_upcasted
			);
		}

		return std::make_shared<value>(
			"__equals", 
			type(type::base::boolean, 0), 
			equals_result
		);
	}

	acceptation_result basic_code_generator::visit_operator_not_equals_node(
		operator_not_equals_node& node, 
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		// accept the left operand
		acceptation_result left_operand_result = node.get_left_expression_node()->accept(
			*this, 
			{}
		);

		if (!left_operand_result.has_value()) {
			return left_operand_result; // return on failure
		}

		// accept the right operand
		acceptation_result right_operand_result = node.get_right_expression_node()->accept(
			*this, 
			{}
		);

		if (!right_operand_result.has_value()) {
			return right_operand_result; // return on failure
		}

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(
			left_operand_result.value()->get_type(),
			right_operand_result.value()->get_type()
		);

		llvm::Value* left_value_upcasted = cast_value(
			left_operand_result.value(), 
			highest_precision,
			node.get_declared_position()
		);

		llvm::Value* right_value_upcasted = cast_value(
			right_operand_result.value(), 
			highest_precision, 
			node.get_declared_position()
		);

		// create a not equals operation based on the highest_precision type
		llvm::Value* not_equals_result;
		if (highest_precision.is_floating_point()) {
			not_equals_result = m_builder.CreateFCmpONE(
				left_value_upcasted, 
				right_value_upcasted
			);
		}
		else {
			not_equals_result = m_builder.CreateICmpNE(
				left_value_upcasted,
				right_value_upcasted
			);
		}

		return std::make_shared<value>(
			"__not_equals", 
			type(type::base::boolean, 0),
			not_equals_result
		);
	}

	std::expected<std::tuple<llvm::Value*, type, acceptation_result>, error_message> basic_code_generator::create_add_operation(
		node* left_operand,
		node* right_operand
	) {
		// accept the left operand (variable to be assigned to)
		acceptation_result left_operand_result = left_operand->accept(
			*this, 
			{}
		);

		if (!left_operand_result.has_value()) {
			return std::unexpected(
				left_operand_result.error()
			); // return on failure
		}

		// accept the right operand
		acceptation_result right_operand_result = right_operand->accept(
			*this, 
			{}
		);

		if (!right_operand_result.has_value()) {
			return std::unexpected(
				right_operand_result.error()
			); // return on failure
		}

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(
			left_operand_result.value()->get_type(),
			right_operand_result.value()->get_type()
		);

		llvm::Value* left_value_upcasted = cast_value(
			left_operand_result.value(), 
			highest_precision, 
			left_operand->get_declared_position()
		);

		llvm::Value* right_value_upcasted = cast_value(
			right_operand_result.value(),
			highest_precision, 
			right_operand->get_declared_position()
		);

		// both types are floating point
		if (highest_precision.is_floating_point()) {
			return std::make_tuple(
				m_builder.CreateFAdd(
					left_value_upcasted,
					right_value_upcasted,
					"fadd"
				),
				highest_precision,
				left_operand_result
			);
		}

		// both types are unsigned
		if (highest_precision.is_unsigned()) {
			return std::make_tuple(
				m_builder.CreateAdd(
					left_value_upcasted,
					right_value_upcasted,
					"uadd",
					true
				),
				highest_precision,
				left_operand_result
			);
		}

		// fallback to regular op
		return std::make_tuple(
			m_builder.CreateAdd(
				left_value_upcasted,
				right_value_upcasted,
				"add"
			), 
			highest_precision,
			left_operand_result
		);
	}

	std::expected<std::tuple<llvm::Value*, type, acceptation_result>, error_message> basic_code_generator::create_sub_operation(
		node* left_operand, 
		node* right_operand
	) {
		// accept the left operand (variable to be assigned to)
		acceptation_result left_operand_result = left_operand->accept(
			*this,
			{}
		);

		if (!left_operand_result.has_value()) {
			return std::unexpected(
				left_operand_result.error()
			); // return on failure
		}

		// accept the right operand
		acceptation_result right_operand_result = right_operand->accept(
			*this, 
			{}
		);

		if (!right_operand_result.has_value()) {
			return std::unexpected(
				right_operand_result.error()
			); // return on failure
		}

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(
			left_operand_result.value()->get_type(),
			right_operand_result.value()->get_type()
		);

		llvm::Value* left_value_upcasted = cast_value(
			left_operand_result.value(), 
			highest_precision, 
			left_operand->get_declared_position()
		);

		llvm::Value* right_value_upcasted = cast_value(
			right_operand_result.value(),
			highest_precision, 
			right_operand->get_declared_position()
		);

		// both types are floating point
		if (highest_precision.is_floating_point()) {
			return std::make_tuple(
				m_builder.CreateFSub(
					left_value_upcasted,
					right_value_upcasted,
					"fsub"
				),
				highest_precision,
				left_operand_result
			);
		}

		// both types are unsigned
		if (highest_precision.is_unsigned()) {
			return std::make_tuple(
				m_builder.CreateSub(
					left_value_upcasted,
					right_value_upcasted,
					"usub",
					true
				),
				highest_precision,
				left_operand_result
			);
		}

		// fallback to regular op
		return std::make_tuple(
			m_builder.CreateSub(
				left_value_upcasted,
				right_value_upcasted,
				"sub"
			),
			highest_precision,
			left_operand_result
		);
	}

	std::expected<std::tuple<llvm::Value*, type, acceptation_result>, error_message> basic_code_generator::create_mul_operation(
		node* left_operand,
		node* right_operand
	) {
		// accept the left operand (variable to be assigned to)
		acceptation_result left_operand_result = left_operand->accept(
			*this, 
			{}
		);

		if (!left_operand_result.has_value()) {
			return std::unexpected(
				left_operand_result.error()
			); // return on failure
		}

		// accept the right operand
		acceptation_result right_operand_result = right_operand->accept(
			*this,
			{}
		);

		if (!right_operand_result.has_value()) {
			return std::unexpected(
				right_operand_result.error()
			); // return on failure
		}

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(
			left_operand_result.value()->get_type(),
			right_operand_result.value()->get_type()
		);

		llvm::Value* left_value_upcasted = cast_value(
			left_operand_result.value(), 
			highest_precision,
			left_operand->get_declared_position()
		);

		llvm::Value* right_value_upcasted = cast_value(
			right_operand_result.value(), 
			highest_precision,
			right_operand->get_declared_position()
		);

		// both types are floating point
		if (highest_precision.is_floating_point()) {
			return std::make_tuple(
				m_builder.CreateFMul(
					left_value_upcasted,
					right_value_upcasted,
					"fmul"
				),
				highest_precision,
				left_operand_result
			);
		}

		// both types are unsigned
		if (highest_precision.is_unsigned()) {
			return std::make_tuple(
				m_builder.CreateMul(
					left_value_upcasted,
					right_value_upcasted,
					"umul",
					true
				),
				highest_precision,
				left_operand_result
			);
		}

		// fallback to regular op
		return std::make_tuple(
			m_builder.CreateMul(
				left_value_upcasted,
				right_value_upcasted,
				"mul"
			),
			highest_precision,
			left_operand_result
		);
	}

	std::expected<std::tuple<llvm::Value*, type, acceptation_result>, error_message> basic_code_generator::create_div_operation(
		node* left_operand,
		node* right_operand
	) {
		// accept the left operand (variable to be assigned to)
		acceptation_result left_operand_result = left_operand->accept(
			*this,
			{}
		);

		if (!left_operand_result.has_value()) {
			return std::unexpected(
				left_operand_result.error()
			); // return on failure
		}

		// accept the right operand
		acceptation_result right_operand_result = right_operand->accept(
			*this, 
			{}
		);

		if (!right_operand_result.has_value()) {
			return std::unexpected(
				right_operand_result.error()
			); // return on failure
		}

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(
			left_operand_result.value()->get_type(),
			right_operand_result.value()->get_type()
		);

		llvm::Value* left_value_upcasted = cast_value(
			left_operand_result.value(),
			highest_precision,
			left_operand->get_declared_position()
		);

		llvm::Value* right_value_upcasted = cast_value(
			right_operand_result.value(),
			highest_precision,
			right_operand->get_declared_position()
		);

		// both types are floating point
		if (highest_precision.is_floating_point()) {
			return std::make_tuple(
				m_builder.CreateFDiv(
					left_value_upcasted,
					right_value_upcasted,
					"fdiv"
				),
				highest_precision,
				left_operand_result
			);
		}

		// both types are unsigned
		if (highest_precision.is_unsigned()) {
			return std::make_tuple(
				m_builder.CreateUDiv(
					left_value_upcasted,
					right_value_upcasted,
					"udiv",
					true
				),
				highest_precision,
				left_operand_result
			);
		}

		// fallback to regular op
		return std::make_tuple(
			m_builder.CreateSDiv(
				left_value_upcasted,
				right_value_upcasted,
				"sdiv"
			),
			highest_precision,
			left_operand_result
		);
	}

	std::expected<std::tuple<llvm::Value*, type, acceptation_result>, error_message> basic_code_generator::create_mod_operation(
		node* left_operand, 
		node* right_operand
	) {
		// accept the left operand (variable to be assigned to)
		acceptation_result left_operand_result = left_operand->accept(
			*this,
			{}
		);

		if (!left_operand_result.has_value()) {
			return std::unexpected(
				left_operand_result.error()
			); // return on failure
		}

		// accept the right operand
		acceptation_result right_operand_result = right_operand->accept(
			*this,
			{}
		);

		if (!right_operand_result.has_value()) {
			return std::unexpected(
				right_operand_result.error()
			); // return on failure
		}

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(
			left_operand_result.value()->get_type(), 
			right_operand_result.value()->get_type()
		);

		llvm::Value* left_value_upcasted = cast_value(
			left_operand_result.value(), 
			highest_precision,
			left_operand->get_declared_position()
		);

		llvm::Value* right_value_upcasted = cast_value(
			right_operand_result.value(),
			highest_precision, 
			right_operand->get_declared_position()
		);

		// both types are floating point
		if (highest_precision.is_floating_point()) {
			return std::make_tuple(
				m_builder.CreateFRem(
					left_value_upcasted,
					right_value_upcasted,
					"fmod"
				),
				highest_precision,
				left_operand_result
			);
		}

		// both types are unsigned
		if (highest_precision.is_unsigned()) {
			return std::make_tuple(
				m_builder.CreateURem(
					left_value_upcasted,
					right_value_upcasted,
					"umod"
				),
				highest_precision,
				left_operand_result
			);
		}

		// fallback to regular op
		return std::make_tuple(
			m_builder.CreateSRem(
				left_value_upcasted,
				right_value_upcasted,
				"smod"
			),
			highest_precision,
			left_operand_result
		);
	}
}