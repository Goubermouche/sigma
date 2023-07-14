#include "code_generator.h"

// unary
// arithmetic
#include "code_generator/abstract_syntax_tree/operators/unary/arithmetic/operator_post_decrement_node.h"
#include "code_generator/abstract_syntax_tree/operators/unary/arithmetic/operator_post_increment_node.h"
#include "code_generator/abstract_syntax_tree/operators/unary/arithmetic/operator_pre_decrement_node.h"
#include "code_generator/abstract_syntax_tree/operators/unary/arithmetic/operator_pre_increment_node.h"
// bitwise
#include "code_generator/abstract_syntax_tree/operators/unary/bitwise/operator_bitwise_not_node.h"
// logical
#include "code_generator/abstract_syntax_tree/operators/unary/logical/operator_not_node.h"
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
// bitwise
#include "code_generator/abstract_syntax_tree/operators/binary/bitwise/operator_bitwise_and_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/bitwise/operator_bitwise_left_shift_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/bitwise/operator_bitwise_or_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/bitwise/operator_bitwise_right_shift_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/bitwise/operator_bitwise_xor_node.h"
// logical
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_conjunction_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_disjunction_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_greater_than_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_greater_than_equal_to_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_less_than_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_less_than_equal_to_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_equals_node.h"
#include "code_generator/abstract_syntax_tree/operators/binary/logical/operator_not_equals_node.h"

namespace sigma {
	// unary
	// arithmetic
	outcome::result<value_ptr> code_generator::visit_operator_post_decrement_node(
		operator_post_decrement_node& node,
		const code_generation_context& context
	) {
		(void)context; // suppress C4100
		OUTCOME_TRY(auto expression_result, node.get_expression_node()->accept(
			*this,
			{}
		));

		// check if the expression is an integer or a floating-point value
		if (!expression_result->get_type().is_numerical()) {
			return outcome::failure(
				error::emit<4100>(
					node.get_declared_location(), 
					expression_result->get_type()
				)
			); // return on failure
		}

		llvm::Value* decrement_result;
		if (expression_result->get_type().is_floating_point()) {
			decrement_result = m_llvm_context->get_builder().CreateFSub(
				expression_result->get_value(),
				llvm::ConstantFP::get(
					expression_result->get_type().get_llvm_type(
						m_llvm_context->get_context()
					), 
					1.0
				)
			);
		}
		else {
			decrement_result = m_llvm_context->get_builder().CreateSub(
				expression_result->get_value(),
				llvm::ConstantInt::get(
					expression_result->get_type().get_llvm_type(m_llvm_context->get_context()),
					1
				)
			);
		}

		m_llvm_context->get_builder().CreateStore(
			decrement_result,
			expression_result->get_pointer()
		);

		return expression_result;
	}

	outcome::result<value_ptr> code_generator::visit_operator_post_increment_node(
		operator_post_increment_node& node,
		const code_generation_context& context
	) {
		(void)context; // suppress C4100
		OUTCOME_TRY(auto expression_result, node.get_expression_node()->accept(
			*this,
			{}
		));

		// check if the expression is an integer or a floating-point value
		if (!expression_result->get_type().is_numerical()) {
			return outcome::failure(
				error::emit<4101>(
					node.get_declared_location(),
					expression_result->get_type()
				)
			); // return on failure
		}

		llvm::Value* decrement_result;
		if (expression_result->get_type().is_floating_point()) {
			decrement_result = m_llvm_context->get_builder().CreateFAdd(
				expression_result->get_value(),
				llvm::ConstantFP::get(
					expression_result->get_type().get_llvm_type(m_llvm_context->get_context()),
					1.0
				)
			);
		}
		else {
			decrement_result = m_llvm_context->get_builder().CreateAdd(
				expression_result->get_value(), 
				llvm::ConstantInt::get(
					expression_result->get_type().get_llvm_type(m_llvm_context->get_context()),
					1
				)
			);
		}

		m_llvm_context->get_builder().CreateStore(
			decrement_result,
			expression_result->get_pointer()
		);

		return expression_result;
	}

	outcome::result<value_ptr> code_generator::visit_operator_pre_decrement_node(
		operator_pre_decrement_node& node, 
		const code_generation_context& context
	) {
		(void)context; // suppress C4100
		// accept the expression
		OUTCOME_TRY(auto expression_result, node.get_expression_node()->accept(
			*this,
			{}
		));

		// check if the expression is an integer or a floating-point value
		if (!expression_result->get_type().is_numerical()) {
			return outcome::failure(
				error::emit<4102>(
					node.get_declared_location(), 
					expression_result->get_type()
				)
			); // return on failure
		}

		// increment the expression
		llvm::Value* increment_result;
		if (expression_result->get_type().is_floating_point()) {
			increment_result = m_llvm_context->get_builder().CreateFSub(
				expression_result->get_value(),
				llvm::ConstantFP::get(
					expression_result->get_type().get_llvm_type(m_llvm_context->get_context()), 
					1.0
				)
			);
		}
		else {
			increment_result = m_llvm_context->get_builder().CreateSub(
				expression_result->get_value(),
				llvm::ConstantInt::get(
					expression_result->get_type().get_llvm_type(m_llvm_context->get_context()),
					1
				)
			);
		}

		// store the decremented value back to memory
		m_llvm_context->get_builder().CreateStore(
			increment_result,
			expression_result->get_pointer()
		);

		return std::make_shared<value>(
			"__pre_increment", 
			expression_result->get_type(),
			increment_result
		);
	}

	outcome::result<value_ptr> code_generator::visit_operator_pre_increment_node(
		operator_pre_increment_node& node, 
		const code_generation_context& context
	) {
		(void)context; // suppress C4100
		// accept the expression
		OUTCOME_TRY(auto expression_result, node.get_expression_node()->accept(
			*this,
			{}
		));

		// check if the expression is an integer or a floating-point value
		if (!expression_result->get_type().is_numerical()) {
			return outcome::failure(
				error::emit<4103>(
					node.get_declared_location(), 
					expression_result->get_type()
				)
			); // return on failure
		}

		// increment the expression
		llvm::Value* increment_result;
		if (expression_result->get_type().is_floating_point()) {
			increment_result = m_llvm_context->get_builder().CreateFAdd(
				expression_result->get_value(), 
				llvm::ConstantFP::get(
					expression_result->get_type().get_llvm_type(m_llvm_context->get_context()),
					1.0
				)
			);
		}
		else {
			increment_result = m_llvm_context->get_builder().CreateAdd(
				expression_result->get_value(),
				llvm::ConstantInt::get(
					expression_result->get_type().get_llvm_type(m_llvm_context->get_context()),
					1
				)
			);
		}

		// store the incremented value back to memory
		m_llvm_context->get_builder().CreateStore(
			increment_result,
			expression_result->get_pointer()
		);

		return std::make_shared<value>(
			"__pre_increment", 
			expression_result->get_type(),
			increment_result
		);
	}

	// bitwise
	outcome::result<value_ptr> code_generator::visit_operator_bitwise_not_node(
		operator_bitwise_not_node& node,
		const code_generation_context& context
	) {
		(void)context; // suppress C4100

		// accept the operand
		OUTCOME_TRY(auto operand_result, node.get_expression_node()->accept(
			*this,
			{}
		));

		// the expression must be integral
		if (!operand_result->get_type().is_integral()) {
			return outcome::failure(
				error::emit<4105>(
					node.get_declared_location(),
					operand_result->get_type()
				)
			); // return on failure
		}

		// create a bitwise NOT operation
		llvm::Value* not_result = m_llvm_context->get_builder().CreateNot(
			operand_result->get_value()
		);

		return std::make_shared<value>(
			"__bitwise_not",
			operand_result->get_type(),
			not_result
		);
	}

	// logical
	outcome::result<value_ptr> code_generator::visit_operator_not_node(
		operator_not_node& node, 
		const code_generation_context& context
	) {
		(void)context; // suppress C4100

		// accept the operand
		OUTCOME_TRY(auto operand_result, node.get_expression_node()->accept(
			*this,
			{}
		));

		if(!operand_result->get_type().is_numerical() && 
			operand_result->get_type().get_base() != type::base::boolean) {
			return outcome::failure(
				error::emit<4104>(
					node.get_declared_location(),
					operand_result->get_type()
				)
			); // return on failure
		}

		// create a not operation (i.e., compare the operand with true and use the result)
		llvm::Value* not_result = m_llvm_context->get_builder().CreateICmpEQ(
			operand_result->get_value(),
			llvm::ConstantInt::get(
				llvm::Type::getInt1Ty(m_llvm_context->get_context()),
				0
			)
		);

		return std::make_shared<value>(
			"__logical_not",
			type(type::base::boolean, 0),
			not_result
		);
	}

	// binary
	// arithmetic
	outcome::result<value_ptr> code_generator::visit_operator_addition_assignment_node(
		operator_addition_assignment_node& node,
		const code_generation_context& context
	) {
		(void)context; // suppress C4100
		OUTCOME_TRY(auto operation_result, create_add_operation(
			node.get_left_expression_node(),
			node.get_right_expression_node()
		));

		const auto& [
			result_value,
			highest_precision, 
			left_operand_result
		] = operation_result;

		// create the assignment value
		auto assignment_value = std::make_shared<value>(
			"__add_assign",
			highest_precision,
			result_value
		);

		// store the result of the addition operation back into the variable
		m_llvm_context->get_builder().CreateStore(
			assignment_value->get_value(), 
			left_operand_result->get_pointer()
		);

		return assignment_value;
	}

	outcome::result<value_ptr> code_generator::visit_operator_addition_node(
		operator_addition_node& node,
		const code_generation_context& context
	) {
		(void)context; // suppress C4100
		OUTCOME_TRY(auto operation_result, create_add_operation(
			node.get_left_expression_node(),
			node.get_right_expression_node()
		));

		const auto& [
			result_value, 
			highest_precision, 
			left_operand_result
		] = operation_result;

		return std::make_shared<value>(
			"__add",
			highest_precision,
			result_value
		);
	}

	outcome::result<value_ptr> code_generator::visit_operator_subtraction_assignment_node(
		operator_subtraction_assignment_node& node,
		const code_generation_context& context
	) {
		(void)context; // suppress C4100
		OUTCOME_TRY(auto operation_result, create_sub_operation(
			node.get_left_expression_node(),
			node.get_right_expression_node()
		));

		const auto& [
			result_value, 
			highest_precision, 
			left_operand_result
		] = operation_result;

		// create the assignment value
		auto assignment_value = std::make_shared<value>(
			"__sub_assign",
			highest_precision,
			result_value
		);

		// store the result of the subtraction operation back into the variable
		m_llvm_context->get_builder().CreateStore(
			assignment_value->get_value(),
			left_operand_result->get_pointer()
		);

		return assignment_value;
	}

	outcome::result<value_ptr> code_generator::visit_operator_subtraction_node(
		operator_subtraction_node& node,
		const code_generation_context& context
	) {
		(void)context; // suppress C4100
		OUTCOME_TRY(auto operation_result, create_sub_operation(
			node.get_left_expression_node(),
			node.get_right_expression_node()
		));

		const auto& [
			result_value,
			highest_precision, 
			left_operand_result
		] = operation_result;

		return std::make_shared<value>(
			"__sub",
			highest_precision,
			result_value
		);
	}

	outcome::result<value_ptr> code_generator::visit_operator_multiplication_assignment_node(
		operator_multiplication_assignment_node& node, 
		const code_generation_context& context
	) {
		(void)context; // suppress C4100
		OUTCOME_TRY(auto operation_result, create_mul_operation(
			node.get_left_expression_node(),
			node.get_right_expression_node()
		));

		const auto& [
			result_value,
			highest_precision,
			left_operand_result
		] = operation_result;

		// create the assignment value
		auto assignment_value = std::make_shared<value>(
			"__sub_assign",
			highest_precision,
			result_value
		);

		// store the result of the multiplication operation back into the variable
		m_llvm_context->get_builder().CreateStore(
			assignment_value->get_value(),
			left_operand_result->get_pointer()
		);

		return assignment_value;
	}

	outcome::result<value_ptr> code_generator::visit_operator_multiplication_node(
		operator_multiplication_node& node,
		const code_generation_context& context
	) {
		(void)context; // suppress C4100
		OUTCOME_TRY(auto operation_result, create_mul_operation(
			node.get_left_expression_node(),
			node.get_right_expression_node()
		));

		const auto& [
			result_value, 
			highest_precision,
			left_operand_result
		] = operation_result;

		return std::make_shared<value>(
			"__mul",
			highest_precision,
			result_value
		);
	}

	outcome::result<value_ptr> code_generator::visit_operator_division_assignment_node(
		operator_division_assignment_node& node, 
		const code_generation_context& context
	) {
		(void)context; // suppress C4100
		OUTCOME_TRY(auto operation_result, create_div_operation(
			node.get_left_expression_node(),
			node.get_right_expression_node()
		));

		const auto& [
			result_value, 
			highest_precision,
			left_operand_result
		] = operation_result;

		// create the assignment value
		auto assignment_value = std::make_shared<value>(
			"__div_assign",
			highest_precision,
			result_value
		);

		// store the result of the division operation back into the variable
		m_llvm_context->get_builder().CreateStore(
			assignment_value->get_value(),
			left_operand_result->get_pointer()
		);

		return assignment_value;
	}

	outcome::result<value_ptr> code_generator::visit_operator_division_node(
		operator_division_node& node, 
		const code_generation_context& context
	) {
		(void)context; // suppress C4100
		OUTCOME_TRY(auto operation_result, create_div_operation(
			node.get_left_expression_node(),
			node.get_right_expression_node()
		));

		const auto& [
			result_value,
			highest_precision, 
			left_operand_result
		] = operation_result;

		return std::make_shared<value>(
			"__div",
			highest_precision,
			result_value
		);
	}

	outcome::result<value_ptr> code_generator::visit_operator_modulo_assignment_node(
		operator_modulo_assignment_node& node,
		const code_generation_context& context
	) {
		(void)context; // suppress C4100
		OUTCOME_TRY(auto operation_result, create_mod_operation(
			node.get_left_expression_node(),
			node.get_right_expression_node()
		));

		const auto& [
			result_value, 
			highest_precision,
			left_operand_result
		] = operation_result;

		// create the assignment value
		auto assignment_value = std::make_shared<value>(
			"__mod_assign",
			highest_precision,
			result_value
		);

		// store the result of the modulo operation back into the variable
		m_llvm_context->get_builder().CreateStore(
			assignment_value->get_value(),
			left_operand_result->get_pointer()
		);

		return assignment_value;
	}

	outcome::result<value_ptr> code_generator::visit_operator_modulo_node(
		operator_modulo_node& node,
		const code_generation_context& context
	) {
		(void)context; // suppress C4100
		OUTCOME_TRY(auto operation_result, create_mod_operation(
			node.get_left_expression_node(),
			node.get_right_expression_node()
		));

		const auto& [
			result_value,
			highest_precision,
			left_operand_result
		] = operation_result;

		return std::make_shared<value>(
			"__mod",
			highest_precision,
			result_value
		);
	}

	outcome::result<value_ptr> code_generator::visit_operator_bitwise_and_node(
		operator_bitwise_and_node& node, 
		const code_generation_context& context
	) {
		(void)context; // suppress C4100

		// accept the left operand
		OUTCOME_TRY(auto left_operand_result, node.get_left_expression_node()->accept(
			*this,
			{}
		));

		// accept the right operand
		OUTCOME_TRY(auto right_operand_result, node.get_right_expression_node()->accept(
			*this,
			{}
		));

		// both expressions must be integral
		if (!left_operand_result->get_type().is_integral() ||
			!right_operand_result->get_type().is_integral()) {
			return outcome::failure(
				error::emit<4202>(
					node.get_declared_location(),
					left_operand_result->get_type(),
					right_operand_result->get_type()
				)
			); // return on failure
		}

		// create a bitwise AND operation
		llvm::Value* and_result = m_llvm_context->get_builder().CreateAnd(
			left_operand_result->get_value(),
			right_operand_result->get_value()
		);

		return std::make_shared<value>(
			"__bitwise_and",
			left_operand_result->get_type(),
			and_result
		);
	}

	outcome::result<value_ptr> code_generator::visit_operator_bitwise_or_node(
		operator_bitwise_or_node& node,
		const code_generation_context& context
	) {
		(void)context; // suppress C4100

		// accept the left operand
		OUTCOME_TRY(auto left_operand_result, node.get_left_expression_node()->accept(
			*this,
			{}
		));

		// accept the right operand
		OUTCOME_TRY(auto right_operand_result, node.get_right_expression_node()->accept(
			*this,
			{}
		));

		// both expressions must be integral
		if (!left_operand_result->get_type().is_integral() ||
			!right_operand_result->get_type().is_integral()) {
			return outcome::failure(
				error::emit<4203>(
					node.get_declared_location(),
					left_operand_result->get_type(),
					right_operand_result->get_type()
				)
			); // return on failure
		}

		// create a bitwise OR operation
		llvm::Value* and_result = m_llvm_context->get_builder().CreateOr(
			left_operand_result->get_value(),
			right_operand_result->get_value()
		);

		return std::make_shared<value>(
			"__bitwise_or",
			left_operand_result->get_type(),
			and_result
		);
	}

	outcome::result<value_ptr> code_generator::visit_operator_bitwise_left_shift_node(
		operator_bitwise_left_shift_node& node, 
		const code_generation_context& context
	) {
		(void)context; // suppress C4100

		// accept the left operand
		OUTCOME_TRY(auto left_operand_result, node.get_left_expression_node()->accept(
			*this,
			{}
		));

		// accept the right operand
		OUTCOME_TRY(auto right_operand_result, node.get_right_expression_node()->accept(
			*this,
			{}
		));

		// both expressions must be integral
		if (!left_operand_result->get_type().is_integral() ||
			!right_operand_result->get_type().is_integral()) {
			return outcome::failure(
				error::emit<4204>(
					node.get_declared_location(),
					left_operand_result->get_type(),
					right_operand_result->get_type()
				)
			); // return on failure
		}

		// create a bitwise left shift operation
		llvm::Value* left_shift_result = m_llvm_context->get_builder().CreateShl(
			left_operand_result->get_value(),
			right_operand_result->get_value()
		);

		return std::make_shared<value>(
			"__bitwise_left_shift",
			left_operand_result->get_type(),
			left_shift_result
		);
	}

	outcome::result<value_ptr> code_generator::visit_operator_bitwise_right_shift_node(
		operator_bitwise_right_shift_node& node,
		const code_generation_context& context
	) {
		(void)context; // suppress C4100

		// accept the left operand
		OUTCOME_TRY(auto left_operand_result, node.get_left_expression_node()->accept(
			*this,
			{}
		));

		// accept the right operand
		OUTCOME_TRY(auto right_operand_result, node.get_right_expression_node()->accept(
			*this,
			{}
		));

		// both expressions must be integral
		if (!left_operand_result->get_type().is_integral() ||
			!right_operand_result->get_type().is_integral()) {
			return outcome::failure(
				error::emit<4205>(
					node.get_declared_location(),
					left_operand_result->get_type(),
					right_operand_result->get_type()
				)
			); // return on failure
		}

		// create a bitwise right shift operation
		llvm::Value* right_shift_result = m_llvm_context->get_builder().CreateLShr(
			left_operand_result->get_value(),
			right_operand_result->get_value()
		);

		return std::make_shared<value>(
			"__bitwise_right_shift",
			left_operand_result->get_type(),
			right_shift_result
		);
	}

	outcome::result<value_ptr> code_generator::visit_operator_bitwise_xor_node(
		operator_bitwise_xor_node& node,
		const code_generation_context& context
	) {
		(void)context; // suppress C4100

		// accept the left operand
		OUTCOME_TRY(auto left_operand_result, node.get_left_expression_node()->accept(
			*this,
			{}
		));

		// accept the right operand
		OUTCOME_TRY(auto right_operand_result, node.get_right_expression_node()->accept(
			*this,
			{}
		));

		// both expressions must be integral
		if (!left_operand_result->get_type().is_integral() ||
			!right_operand_result->get_type().is_integral()) {
			return outcome::failure(
				error::emit<4206>(
					node.get_declared_location(),
					left_operand_result->get_type(),
					right_operand_result->get_type()
				)
			); // return on failure
		}

		// create a bitwise XOR operation
		llvm::Value* xor_result = m_llvm_context->get_builder().CreateXor(
			left_operand_result->get_value(),
			right_operand_result->get_value()
		);

		return std::make_shared<value>(
			"__bitwise_xor",
			left_operand_result->get_type(),
			xor_result
		);
	}

	// logical
	outcome::result<value_ptr> code_generator::visit_operator_logical_conjunction_node(
		operator_conjunction_node& node,
		const code_generation_context& context
	) {
		(void)context; // suppress C4100

		// accept the left operand
		OUTCOME_TRY(auto left_operand_result, node.get_left_expression_node()->accept(
			*this,
			{}
		));

		// accept the right operand
		OUTCOME_TRY(auto right_operand_result, node.get_right_expression_node()->accept(
			*this,
			{}
		));

		// both expressions must be boolean
		if (left_operand_result->get_type().get_base() != type::base::boolean || 
			right_operand_result->get_type().get_base() != type::base::boolean) {
			return outcome::failure(
				error::emit<4200>(
					node.get_declared_location(),
					left_operand_result->get_type(),
					right_operand_result->get_type()
				)
			); // return on failure
		}

		// create a logical AND operation
		llvm::Value* and_result = m_llvm_context->get_builder().CreateAnd(
			left_operand_result->get_value(),
			right_operand_result->get_value(),
			"and"
		);

		return std::make_shared<value>(
			"__logical_conjunction", 
			type(type::base::boolean, 0),
			and_result
		);
	}

	outcome::result<value_ptr> code_generator::visit_operator_logical_disjunction_node(
		operator_disjunction_node& node, 
		const code_generation_context& context
	) {
		(void)context; // suppress C4100

		// accept the left operand
		OUTCOME_TRY(auto left_operand_result, node.get_left_expression_node()->accept(
			*this,
			{}
		));

		// accept the right operand
		OUTCOME_TRY(auto right_operand_result, node.get_right_expression_node()->accept(
			*this,
			{}
		));

		// both expressions must be boolean
		if (left_operand_result->get_type().get_base() != type::base::boolean ||
			right_operand_result->get_type().get_base() != type::base::boolean) {
			return outcome::failure(
				error::emit<4201>(
					node.get_declared_location(),
					left_operand_result->get_type(), 
					right_operand_result->get_type()
				)
			); // return on failure
		}

		// create a logical OR operation
		llvm::Value* or_result = m_llvm_context->get_builder().CreateOr(
			left_operand_result->get_value(),
			right_operand_result->get_value()
		);

		return std::make_shared<value>(
			"__logical_disjunction",
			type(type::base::boolean, 0),
			or_result
		);
	}

	outcome::result<value_ptr> code_generator::visit_operator_greater_than_node(
		operator_greater_than_node& node, 
		const code_generation_context& context
	) {
		(void)context; // suppress C4100

		// accept the left operand
		OUTCOME_TRY(auto left_operand_result, node.get_left_expression_node()->accept(
			*this,
			{}
		));

		// accept the right operand
		OUTCOME_TRY(auto right_operand_result, node.get_right_expression_node()->accept(
			*this,
			{}
		));

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(
			left_operand_result->get_type(),
			right_operand_result->get_type()
		);

		llvm::Value* left_value_upcasted = cast_value(
			left_operand_result,
			highest_precision,
			node.get_declared_location()
		);

		llvm::Value* right_value_upcasted = cast_value(
			right_operand_result, 
			highest_precision, 
			node.get_declared_location()
		);

		// create a greater than operation based on the highest_precision type
		llvm::Value* greater_than_result;
		if (highest_precision.is_floating_point()) {
			greater_than_result = m_llvm_context->get_builder().CreateFCmpOGT(
				left_value_upcasted,
				right_value_upcasted
			);
		}
		else {
			if (highest_precision.is_unsigned()) {
				greater_than_result = m_llvm_context->get_builder().CreateICmpUGT(
					left_value_upcasted, 
					right_value_upcasted
				);
			}
			else {
				greater_than_result = m_llvm_context->get_builder().CreateICmpSGT(
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

	outcome::result<value_ptr> code_generator::visit_operator_greater_than_equal_to_node(
		operator_greater_than_equal_to_node& node, 
		const code_generation_context& context
	) {
		(void)context; // suppress C4100

		// accept the left operand
		OUTCOME_TRY(auto left_operand_result, node.get_left_expression_node()->accept(
			*this,
			{}
		));

		// accept the right operand
		OUTCOME_TRY(auto right_operand_result, node.get_right_expression_node()->accept(
			*this,
			{}
		));

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(
			left_operand_result->get_type(),
			right_operand_result->get_type()
		);

		llvm::Value* left_value_upcasted = cast_value(
			left_operand_result,
			highest_precision,
			node.get_declared_location()
		);

		llvm::Value* right_value_upcasted = cast_value(
			right_operand_result, 
			highest_precision,
			node.get_declared_location()
		);

		// create a greater than or equal to operation based on the highest_precision type
		llvm::Value* greater_than_equal_result;
		if (highest_precision.is_floating_point()) {
			greater_than_equal_result = m_llvm_context->get_builder().CreateFCmpOGE(
				left_value_upcasted, 
				right_value_upcasted
			);
		}
		else {
			if (highest_precision.is_unsigned()) {
				greater_than_equal_result = m_llvm_context->get_builder().CreateICmpUGE(
					left_value_upcasted, 
					right_value_upcasted
				);
			}
			else {
				greater_than_equal_result = m_llvm_context->get_builder().CreateICmpSGE(
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

	outcome::result<value_ptr> code_generator::visit_operator_less_than_node(
		operator_less_than_node& node, 
		const code_generation_context& context
	) {
		(void)context; // suppress C4100

		// accept the left operand
		OUTCOME_TRY(auto left_operand_result, node.get_left_expression_node()->accept(
			*this,
			{}
		));

		// accept the right operand
		OUTCOME_TRY(auto right_operand_result, node.get_right_expression_node()->accept(
			*this,
			{}
		));

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(
			left_operand_result->get_type(),
			right_operand_result->get_type()
		);

		llvm::Value* left_value_upcasted = cast_value(
			left_operand_result,
			highest_precision,
			node.get_declared_location()
		);

		llvm::Value* right_value_upcasted = cast_value(
			right_operand_result, 
			highest_precision,
			node.get_declared_location()
		);

		// create a less than operation based on the highest_precision type
		llvm::Value* less_than_result;
		if (highest_precision.is_floating_point()) {
			less_than_result = m_llvm_context->get_builder().CreateFCmpOLT(
				left_value_upcasted, 
				right_value_upcasted
			);
		}
		else {
			if (highest_precision.is_unsigned()) {
				less_than_result = m_llvm_context->get_builder().CreateICmpULT(
					left_value_upcasted,
					right_value_upcasted
				);
			}
			else {
				less_than_result = m_llvm_context->get_builder().CreateICmpSLT(
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

	outcome::result<value_ptr> code_generator::visit_operator_less_than_equal_to_node(
		operator_less_than_equal_to_node& node,
		const code_generation_context& context
	) {
		(void)context; // suppress C4100

		// accept the left operand
		OUTCOME_TRY(auto left_operand_result, node.get_left_expression_node()->accept(
			*this,
			{}
		));

		// accept the right operand
		OUTCOME_TRY(auto right_operand_result, node.get_right_expression_node()->accept(
			*this,
			{}
		));

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(
			left_operand_result->get_type(),
			right_operand_result->get_type()
		);

		llvm::Value* left_value_upcasted = cast_value(
			left_operand_result,
			highest_precision,
			node.get_declared_location()
		);

		llvm::Value* right_value_upcasted = cast_value(
			right_operand_result, 
			highest_precision, 
			node.get_declared_location()
		);

		// create a less than or equal to operation based on the highest_precision type
		llvm::Value* less_than_equal_result;
		if (highest_precision.is_floating_point()) {
			less_than_equal_result = m_llvm_context->get_builder().CreateFCmpOLE(
				left_value_upcasted,
				right_value_upcasted
			);
		}
		else {
			if (highest_precision.is_unsigned()) {
				less_than_equal_result = m_llvm_context->get_builder().CreateICmpULE(
					left_value_upcasted,
					right_value_upcasted
				);
			}
			else {
				less_than_equal_result = m_llvm_context->get_builder().CreateICmpSLE(
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

	outcome::result<value_ptr> code_generator::visit_operator_equals_node(
		operator_equals_node& node, 
		const code_generation_context& context
	) {
		(void)context; // suppress C4100

		// accept the left operand
		OUTCOME_TRY(auto left_operand_result, node.get_left_expression_node()->accept(
			*this,
			{}
		));

		// accept the right operand
		OUTCOME_TRY(auto right_operand_result, node.get_right_expression_node()->accept(
			*this,
			{}
		));

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(
			left_operand_result->get_type(),
			right_operand_result->get_type()
		);

		llvm::Value* left_value_upcasted = cast_value(
			left_operand_result,
			highest_precision,
			node.get_declared_location()
		);

		llvm::Value* right_value_upcasted = cast_value(
			right_operand_result,
			highest_precision,
			node.get_declared_location()
		);

		// create an equals operation based on the highest_precision type
		llvm::Value* equals_result;
		if (highest_precision.is_floating_point()) {
			equals_result = m_llvm_context->get_builder().CreateFCmpOEQ(
				left_value_upcasted, 
				right_value_upcasted
			);
		}
		else {
			equals_result = m_llvm_context->get_builder().CreateICmpEQ(
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

	outcome::result<value_ptr> code_generator::visit_operator_not_equals_node(
		operator_not_equals_node& node, 
		const code_generation_context& context
	) {
		(void)context; // suppress C4100

		// accept the left operand
		OUTCOME_TRY(auto left_operand_result, node.get_left_expression_node()->accept(
			*this,
			{}
		));

		// accept the right operand
		OUTCOME_TRY(auto right_operand_result, node.get_right_expression_node()->accept(
			*this,
			{}
		));

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(
			left_operand_result->get_type(),
			right_operand_result->get_type()
		);

		llvm::Value* left_value_upcasted = cast_value(
			left_operand_result, 
			highest_precision,
			node.get_declared_location()
		);

		llvm::Value* right_value_upcasted = cast_value(
			right_operand_result, 
			highest_precision, 
			node.get_declared_location()
		);

		// create a not equals operation based on the highest_precision type
		llvm::Value* not_equals_result;
		if (highest_precision.is_floating_point()) {
			not_equals_result = m_llvm_context->get_builder().CreateFCmpONE(
				left_value_upcasted, 
				right_value_upcasted
			);
		}
		else {
			not_equals_result = m_llvm_context->get_builder().CreateICmpNE(
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

	outcome::result<std::tuple<llvm::Value*, type, value_ptr>> code_generator::create_add_operation(
		node_ptr left_operand,
		node_ptr right_operand
	) {
		// accept the left operand (variable to be assigned to)
		OUTCOME_TRY(auto left_operand_result, left_operand->accept(
			*this,
			{}
		));

		// accept the right operand
		OUTCOME_TRY(auto right_operand_result, right_operand->accept(
			*this,
			{}
		));

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(
			left_operand_result->get_type(),
			right_operand_result->get_type()
		);

		llvm::Value* left_value_upcasted = cast_value(
			left_operand_result, 
			highest_precision, 
			left_operand->get_declared_location()
		);

		llvm::Value* right_value_upcasted = cast_value(
			right_operand_result,
			highest_precision, 
			right_operand->get_declared_location()
		);

		// both types are floating point
		if (highest_precision.is_floating_point()) {
			return std::make_tuple(
				m_llvm_context->get_builder().CreateFAdd(
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
				m_llvm_context->get_builder().CreateAdd(
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
			m_llvm_context->get_builder().CreateAdd(
				left_value_upcasted,
				right_value_upcasted,
				"add"
			), 
			highest_precision,
			left_operand_result
		);
	}

	outcome::result<std::tuple<llvm::Value*, type, value_ptr>> code_generator::create_sub_operation(
		node_ptr left_operand, 
		node_ptr right_operand
	) {
		// accept the left operand (variable to be assigned to)
		OUTCOME_TRY(auto left_operand_result, left_operand->accept(
			*this,
			{}
		));

		// accept the right operand
		OUTCOME_TRY(auto right_operand_result, right_operand->accept(
			*this,
			{}
		));

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(
			left_operand_result->get_type(),
			right_operand_result->get_type()
		);

		llvm::Value* left_value_upcasted = cast_value(
			left_operand_result, 
			highest_precision, 
			left_operand->get_declared_location()
		);

		llvm::Value* right_value_upcasted = cast_value(
			right_operand_result,
			highest_precision, 
			right_operand->get_declared_location()
		);

		// both types are floating point
		if (highest_precision.is_floating_point()) {
			return std::make_tuple(
				m_llvm_context->get_builder().CreateFSub(
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
				m_llvm_context->get_builder().CreateSub(
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
			m_llvm_context->get_builder().CreateSub(
				left_value_upcasted,
				right_value_upcasted,
				"sub"
			),
			highest_precision,
			left_operand_result
		);
	}

	outcome::result<std::tuple<llvm::Value*, type, value_ptr>> code_generator::create_mul_operation(
		node_ptr left_operand,
		node_ptr right_operand
	) {
		// accept the left operand (variable to be assigned to)
		OUTCOME_TRY(auto left_operand_result, left_operand->accept(
			*this,
			{}
		));

		// accept the right operand
		OUTCOME_TRY(auto right_operand_result, right_operand->accept(
			*this,
			{}
		));

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(
			left_operand_result->get_type(),
			right_operand_result->get_type()
		);

		llvm::Value* left_value_upcasted = cast_value(
			left_operand_result, 
			highest_precision,
			left_operand->get_declared_location()
		);

		llvm::Value* right_value_upcasted = cast_value(
			right_operand_result, 
			highest_precision,
			right_operand->get_declared_location()
		);

		// both types are floating point
		if (highest_precision.is_floating_point()) {
			return std::make_tuple(
				m_llvm_context->get_builder().CreateFMul(
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
				m_llvm_context->get_builder().CreateMul(
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
			m_llvm_context->get_builder().CreateMul(
				left_value_upcasted,
				right_value_upcasted,
				"mul"
			),
			highest_precision,
			left_operand_result
		);
	}

	outcome::result<std::tuple<llvm::Value*, type, value_ptr>> code_generator::create_div_operation(
		node_ptr left_operand,
		node_ptr right_operand
	) {
		// accept the left operand (variable to be assigned to)
		OUTCOME_TRY(auto left_operand_result, left_operand->accept(
			*this,
			{}
		));

		// accept the right operand
		OUTCOME_TRY(auto right_operand_result, right_operand->accept(
			*this,
			{}
		));

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(
			left_operand_result->get_type(),
			right_operand_result->get_type()
		);

		llvm::Value* left_value_upcasted = cast_value(
			left_operand_result,
			highest_precision,
			left_operand->get_declared_location()
		);

		llvm::Value* right_value_upcasted = cast_value(
			right_operand_result,
			highest_precision,
			right_operand->get_declared_location()
		);

		// both types are floating point
		if (highest_precision.is_floating_point()) {
			return std::make_tuple(
				m_llvm_context->get_builder().CreateFDiv(
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
				m_llvm_context->get_builder().CreateUDiv(
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
			m_llvm_context->get_builder().CreateSDiv(
				left_value_upcasted,
				right_value_upcasted,
				"sdiv"
			),
			highest_precision,
			left_operand_result
		);
	}

	outcome::result<std::tuple<llvm::Value*, type, value_ptr>> code_generator::create_mod_operation(
		node_ptr left_operand, 
		node_ptr right_operand
	) {
		// accept the left operand (variable to be assigned to)
		OUTCOME_TRY(auto left_operand_result, left_operand->accept(
			*this,
			{}
		));

		// accept the right operand
		OUTCOME_TRY(auto right_operand_result, right_operand->accept(
			*this,
			{}
		));

		// upcast both expressions
		const type highest_precision = get_highest_precision_type(
			left_operand_result->get_type(), 
			right_operand_result->get_type()
		);

		llvm::Value* left_value_upcasted = cast_value(
			left_operand_result, 
			highest_precision,
			left_operand->get_declared_location()
		);

		llvm::Value* right_value_upcasted = cast_value(
			right_operand_result,
			highest_precision, 
			right_operand->get_declared_location()
		);

		// both types are floating point
		if (highest_precision.is_floating_point()) {
			return std::make_tuple(
				m_llvm_context->get_builder().CreateFRem(
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
				m_llvm_context->get_builder().CreateURem(
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
			m_llvm_context->get_builder().CreateSRem(
				left_value_upcasted,
				right_value_upcasted,
				"smod"
			),
			highest_precision,
			left_operand_result
		);
	}
}
