#include "codegen_visitor.h"

// variables
#include "abstract_syntax_tree/variables/variable_node.h"
#include "abstract_syntax_tree/variables/declaration/declaration_node.h"
#include "abstract_syntax_tree/variables/declaration/local_declaration_node.h"
#include "abstract_syntax_tree/variables/declaration/global_declaration_node.h"

// keywords
#include "abstract_syntax_tree/variables/assignment_node.h"
#include "abstract_syntax_tree/variables/function_call_node.h"
#include "abstract_syntax_tree/variables/function_node.h"
#include "abstract_syntax_tree/keywords/return_node.h"

// types 
#include "abstract_syntax_tree/keywords/types/signed_int/keyword_i8_node.h"
#include "abstract_syntax_tree/keywords/types/signed_int/keyword_i16_node.h"
#include "abstract_syntax_tree/keywords/types/signed_int/keyword_i32_node.h"
#include "abstract_syntax_tree/keywords/types/signed_int/keyword_i64_node.h"

#include "abstract_syntax_tree/keywords/types/unsigned_int/keyword_u8_node.h"
#include "abstract_syntax_tree/keywords/types/unsigned_int/keyword_u16_node.h"
#include "abstract_syntax_tree/keywords/types/unsigned_int/keyword_u32_node.h"
#include "abstract_syntax_tree/keywords/types/unsigned_int/keyword_u64_node.h"

#include "abstract_syntax_tree/keywords/types/floating_point/keyword_f32_node.h"
#include "abstract_syntax_tree/keywords/types/floating_point/keyword_f64_node.h"

// operators
#include "abstract_syntax_tree/operators/operator_addition_node.h"
#include "abstract_syntax_tree/operators/operator_subtraction_node.h"
#include "abstract_syntax_tree/operators/operator_multiplication_node.h"
#include "abstract_syntax_tree/operators/operator_division_node.h"
#include "abstract_syntax_tree/operators/operator_modulo_node.h"

#include <llvm/IR/Verifier.h>

namespace channel {
	codegen_visitor::codegen_visitor()
		: m_scope(new scope(nullptr)), m_builder(m_context) {
		m_module = std::make_unique<llvm::Module>("channel", m_context);
	}

	bool codegen_visitor::generate(parser& parser) {
		// parse the source file
		std::vector<node*> abstract_syntax_tree;
		if (!parser.parse(abstract_syntax_tree)) {
			return false; // parsing failed, return false
		}

		// walk the abstract syntax tree
		for (node* node : abstract_syntax_tree) {
			node->accept(*this);
		}

		initialize_global_variables();
		std::cout << "----------------------------\n";
		return true;
	}

	void codegen_visitor::initialize_global_variables() {
		// create the global ctors array
		llvm::ArrayType* updated_ctor_array_type = llvm::ArrayType::get(CTOR_STRUCT_TYPE, m_ctors.size());
		llvm::Constant* updated_ctors = llvm::ConstantArray::get(updated_ctor_array_type, m_ctors);
		new llvm::GlobalVariable(*m_module, updated_ctor_array_type, false, llvm::GlobalValue::AppendingLinkage, updated_ctors, "llvm.global_ctors");
	}

	void codegen_visitor::print_intermediate_representation() const {
		m_module->print(llvm::outs(), nullptr);
	}

	void codegen_visitor::verify_intermediate_representation() const {
		// check if we have a 'main' function
		ASSERT(has_main_entry_point(), "[codegen]: cannot find main entrypoint");

		// check for IR errors
		if (!llvm::verifyModule(*m_module, &llvm::outs())) {
			std::cout << "[codegen]: all checks passed\n";
		}
	}

	value* codegen_visitor::visit_assignment_node(assignment_node& node) {
		// local variable
		if (const value* local_variable = m_scope->get_named_value(node.get_name())) {
			// evaluate the expression on the right-hand side of the assignment
			value* new_value = node.get_expression()->accept(*this);

			// upcast the new_value if necessary
			new_value->set_value(cast_value(new_value, local_variable->get_type(), node.get_declaration_line_index()));

			// store the value in the memory location of the variable
			m_builder.CreateStore(new_value->get_value(), local_variable->get_value());
			return new_value;
		}

		// global variable
		// look up the global variable in the m_global_named_values map
		const value* pointer_to_global_variable = m_global_named_values[node.get_name()];
		ASSERT(pointer_to_global_variable, "[codegen]: variable '" + node.get_name() + "' not found");

		// evaluate the expression on the right-hand side of the assignment
		value* new_value = node.get_expression()->accept(*this);

		// upcast the new_value if necessary
		new_value->set_value(cast_value(new_value, pointer_to_global_variable->get_type(), node.get_declaration_line_index()));

		// store the new value in the memory location of the global variable
		m_builder.CreateStore(new_value->get_value(), pointer_to_global_variable->get_value());
		return new_value;
	}

	value* codegen_visitor::visit_function_call_node(function_call_node& node) {
		llvm::Function* function = m_functions[node.get_name()]->get_function();
		ASSERT(function, "[codegen]: function '" + node.get_name() + "' cannot be found");

		// generate code for each argument expression
		std::vector<llvm::Value*> argument_values;
		for (channel::node* argument : node.get_arguments()) {
			argument->accept(*this);
			argument_values.push_back(m_builder.GetInsertBlock()->getParent()->back().getTerminator()->getOperand(0));
		}

		return new value(node.get_name(), type::function_call, m_builder.CreateCall(function, argument_values, "call"));
	}

	value* codegen_visitor::visit_variable_node(variable_node& node) {
		// local variable
		if (const value* variable_value = m_scope->get_named_value(node.get_name())) {
			// load the value from the memory location
			const llvm::AllocaInst* alloca = llvm::dyn_cast<llvm::AllocaInst>(variable_value->get_value());

			llvm::Value* load = m_builder.CreateLoad(
				alloca->getAllocatedType(),
				variable_value->get_value(), 
				node.get_name()
			);

			return new value(node.get_name(), variable_value->get_type(), load);
		}

		// global variable
		const value* pointer_to_global_variable = m_global_named_values[node.get_name()];
		ASSERT(pointer_to_global_variable, "[codegen]: variable '" + node.get_name() + "' not found");
		const llvm::GlobalValue* global_variable_value = llvm::dyn_cast<llvm::GlobalValue>(pointer_to_global_variable->get_value());

		llvm::Value* load = m_builder.CreateLoad(
			global_variable_value->getValueType(),
			pointer_to_global_variable->get_value(),
			node.get_name()
		);

		return new value(node.get_name(), pointer_to_global_variable->get_type(), load);
	}

	value* codegen_visitor::visit_function_node(function_node& node) {
		llvm::Type* return_type = type_to_llvm_type(node.get_return_type(), m_context);
		llvm::FunctionType* function_type = llvm::FunctionType::get(return_type, false);
		llvm::Function* func = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, node.get_name(), m_module.get());

		const auto insertion_result = m_functions.insert({
			node.get_name(),
			new function(node.get_return_type(), func)
		});

		ASSERT(insertion_result.second, "[codegen]: function '" + node.get_name() + "' has already been defined");

		llvm::BasicBlock* entry_block = llvm::BasicBlock::Create(m_context, "entry", func);
		m_builder.SetInsertPoint(entry_block);

		// create a new nested scope for the function body
		scope* prev_scope = m_scope;
		m_scope = new scope(prev_scope);

		// accept all statements inside the function
		for (const auto& statement : node.get_statements()) {
			statement->accept(*this);
		}

		// restore the previous scope
		m_scope = prev_scope;

		// add a return statement if the function does not have one
		if (entry_block->getTerminator() == nullptr) {
			if (return_type->isVoidTy()) {
				m_builder.CreateRetVoid();
			}
			else {
				m_builder.CreateRet(llvm::Constant::getNullValue(return_type));
			}
		}

		return new value(node.get_name(), type::function, func);
	}

	value* codegen_visitor::visit_return_node(return_node& node) {
		// evaluate the expression of the return statement
		const value* return_value = node.get_expression()->accept(*this);

		// get the return type of the current function
		const type function_return_type = m_functions[m_builder.GetInsertBlock()->getParent()->getName().str()]->get_return_type();

		// upcast the return value to match the function's return type
		llvm::Value* upcasted_return_value = cast_value(return_value, function_return_type, node.get_declaration_line_index());

		// generate the LLVM return instruction with the upcasted value
		m_builder.CreateRet(upcasted_return_value);

		// return the value of the expression (use upcasted value's type)
		return new value("__return", function_return_type, upcasted_return_value);
	}

	value* codegen_visitor::visit_local_declaration_node(local_declaration_node& node) {
		// evaluate the assigned value, if there is one
		value* assigned_value = get_declaration_value(node);

		// const type highest_precision = get_highest_precision_type(node.get_declaration_type(), assigned_value->get_type());
		llvm::Value* upcasted_assigned_value = cast_value(assigned_value, node.get_declaration_type(), node.get_declaration_line_index());

		// store the initial value in the memory
		llvm::AllocaInst* alloca = m_builder.CreateAlloca(
			type_to_llvm_type(node.get_declaration_type(), m_context),
			nullptr,
			node.get_name()
		);

		m_builder.CreateStore(upcasted_assigned_value, alloca);

		// add the variable to the current scope
		ASSERT(!m_global_named_values[node.get_name()], "[codegen]: local variable '" + node.get_name() + "' has alredy been defined in the global scope");
		const auto insertion_result = m_scope->add_named_value(node.get_name(), new value(node.get_name(), node.get_declaration_type(), alloca));
		ASSERT(insertion_result.second, "[codegen]: local variable '" + node.get_name() + "' has already been defined before");
		return assigned_value;
	}

	value* codegen_visitor::visit_global_declaration_node(global_declaration_node& node) {
		// start creating the init function
		const std::string init_func_name = "__global_init_" + node.get_name();
		llvm::FunctionType* init_func_type = llvm::FunctionType::get(llvm::Type::getVoidTy(m_context), false);
		llvm::Function* init_func = llvm::Function::Create(init_func_type, llvm::Function::InternalLinkage, init_func_name, m_module.get());
		llvm::BasicBlock* init_func_entry = llvm::BasicBlock::Create(m_context, "entry", init_func);
		m_builder.SetInsertPoint(init_func_entry); // write to the init function

		// evaluate the assigned value, if there is one
		const value* assigned_value = get_declaration_value(node);

		// const type highest_precision = get_highest_precision_type(node.get_declaration_type(), initial_value->get_type());
		llvm::Value* upcasted_initial_value = cast_value(assigned_value, node.get_declaration_type(), node.get_declaration_line_index());

		// create a global variable
		auto* global_variable = new value(
			node.get_name(),
			node.get_declaration_type(),
			new llvm::GlobalVariable(*m_module,
				type_to_llvm_type(node.get_declaration_type(), m_context),
				false,
				llvm::GlobalValue::ExternalLinkage,
				llvm::Constant::getNullValue(type_to_llvm_type(node.get_declaration_type(), m_context)), // default initializer
				node.get_name()
			)
		);

		// add the variable to the m_global_named_values map
		const auto insertion_result = m_global_named_values.insert({ node.get_name(),  global_variable });
		ASSERT(insertion_result.second, "[codegen]: global variable '" + node.get_name() + "' has already been defined before");

		m_builder.CreateStore(upcasted_initial_value, global_variable->get_value());
		m_builder.CreateRetVoid();

		// create a new constructor with the given priority
		llvm::ConstantInt* priority = llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), m_global_initialization_priority++);
		llvm::Constant* initializer_cast = llvm::ConstantExpr::getBitCast(init_func, llvm::Type::getInt8PtrTy(m_context));
		llvm::Constant* new_ctor = llvm::ConstantStruct::get(CTOR_STRUCT_TYPE, {
			priority,
			initializer_cast,
			llvm::Constant::getNullValue(llvm::Type::getInt8PtrTy(m_context))
		});

		m_ctors.push_back(new_ctor);
		return global_variable;
	}

	value* codegen_visitor::visit_keyword_i8_node(keyword_i8_node& node) {
		return new value("__i8", type::i8, llvm::ConstantInt::get(m_context, llvm::APInt(8, node.get_value(), true)));
	}

	value* codegen_visitor::visit_keyword_i16_node(keyword_i16_node& node) {
		return new value("__i16", type::i16, llvm::ConstantInt::get(m_context, llvm::APInt(16, node.get_value(), true)));
	}

	value* codegen_visitor::visit_keyword_i32_node(keyword_i32_node& node) {
		return new value("__i32", type::i32, llvm::ConstantInt::get(m_context, llvm::APInt(32, node.get_value(), true)));
	}

	value* codegen_visitor::visit_keyword_i64_node(keyword_i64_node& node) {
		return new value("__i64", type::i64, llvm::ConstantInt::get(m_context, llvm::APInt(64, node.get_value(), true)));
	}

	value* codegen_visitor::visit_keyword_u8_node(keyword_u8_node& node) {
		return new value("__u8", type::u8, llvm::ConstantInt::get(m_context, llvm::APInt(8, node.get_value(), false)));
	}

	value* codegen_visitor::visit_keyword_u16_node(keyword_u16_node& node) {
		return new value("__u16", type::u16, llvm::ConstantInt::get(m_context, llvm::APInt(16, node.get_value(), false)));
	}

	value* codegen_visitor::visit_keyword_u32_node(keyword_u32_node& node) {
		return new value("__u32", type::u32, llvm::ConstantInt::get(m_context, llvm::APInt(32, node.get_value(), false)));
	}

	value* codegen_visitor::visit_keyword_u64_node(keyword_u64_node& node) {
		return new value("__u64", type::u64, llvm::ConstantInt::get(m_context, llvm::APInt(64, node.get_value(), false)));
	}

	value* codegen_visitor::visit_keyword_f32_node(keyword_f32_node& node) {
		return new value("__f32", type::f32, llvm::ConstantFP::get(m_context, llvm::APFloat(node.get_value())));
	}

	value* codegen_visitor::visit_keyword_f64_node(keyword_f64_node& node) {
		return new value("__f64", type::f64, llvm::ConstantFP::get(m_context, llvm::APFloat(node.get_value())));
	}

	value* codegen_visitor::visit_operator_addition_node(operator_addition_node& node) {
		const value* left = node.left->accept(*this);
		const value* right = node.right->accept(*this);

		const type highest_precision = get_highest_precision_type(left->get_type(), right->get_type());
		llvm::Value* left_value_upcasted = cast_value(left, highest_precision, node.get_declaration_line_index());
		llvm::Value* right_value_upcasted = cast_value(right, highest_precision, node.get_declaration_line_index());

		if (is_type_floating_point(highest_precision)) {
			return new value(
				"__fadd",
				highest_precision,
				m_builder.CreateFAdd(
					left_value_upcasted, 
					right_value_upcasted, 
					"fadd")
			);
		}

		if (is_type_unsigned(highest_precision)) {
			return new value(
				"__uadd",
				highest_precision,
				m_builder.CreateAdd(
					left_value_upcasted, 
					right_value_upcasted, 
					"uadd", 
					true)
			);
		}

		return new value(
			"__add",
			highest_precision, 
			m_builder.CreateAdd(
				left_value_upcasted,
				right_value_upcasted,
				"add")
		);
	}

	value* codegen_visitor::visit_operator_subtraction_node(operator_subtraction_node& node) {
		const value* left = node.left->accept(*this);
		const value* right = node.right->accept(*this);

		const type highest_precision = get_highest_precision_type(left->get_type(), right->get_type());
		llvm::Value* left_value_upcasted = cast_value(left, highest_precision, node.get_declaration_line_index());
		llvm::Value* right_value_upcasted = cast_value(right, highest_precision, node.get_declaration_line_index());

		if (is_type_floating_point(highest_precision)) {
			return new value(
				"__fsub",
				highest_precision, 
				m_builder.CreateFSub(
					left_value_upcasted,
					right_value_upcasted, 
					"fsub")
			);
		}

		if (is_type_unsigned(highest_precision)) {
			return new value(
				"__usub",
				highest_precision, 
				m_builder.CreateSub(
					left_value_upcasted, 
					right_value_upcasted, 
					"usub", 
					true)
			);
		}

		return new value(
			"__sub",
			highest_precision, 
			m_builder.CreateSub(
				left_value_upcasted,
				right_value_upcasted, 
				"sub")
		);
	}

	value* codegen_visitor::visit_operator_multiplication_node(operator_multiplication_node& node) {
		const value* left = node.left->accept(*this);
		const value* right = node.right->accept(*this);

		const type highest_precision = get_highest_precision_type(left->get_type(), right->get_type());
		llvm::Value* left_value_upcasted = cast_value(left, highest_precision, node.get_declaration_line_index());
		llvm::Value* right_value_upcasted = cast_value(right, highest_precision, node.get_declaration_line_index());

		if (is_type_floating_point(highest_precision)) {
			return new value(
				"__fmul",
				highest_precision, 
				m_builder.CreateFMul(
					left_value_upcasted, 
					right_value_upcasted,
					"fmul")
			);
		}

		if (is_type_unsigned(highest_precision)) {
			return new value(
				"__umul",
				highest_precision, 
				m_builder.CreateMul(
					left_value_upcasted,
					right_value_upcasted,
					"umul", 
					true)
			);
		}

		return new value(
			"__mul",
			highest_precision, 
			m_builder.CreateMul(
				left_value_upcasted, 
				right_value_upcasted, 
				"mul")
		);
	}

	value* codegen_visitor::visit_operator_division_node(operator_division_node& node) {
		const value* left = node.left->accept(*this);
		const value* right = node.right->accept(*this);

		const type highest_precision = get_highest_precision_type(left->get_type(), right->get_type());
		llvm::Value* left_value_upcasted = cast_value(left, highest_precision, node.get_declaration_line_index());
		llvm::Value* right_value_upcasted = cast_value(right, highest_precision, node.get_declaration_line_index());

		if (is_type_floating_point(highest_precision)) {
			return new value(
				"__fdiv",
				highest_precision, 
				m_builder.CreateFDiv(
					left_value_upcasted, 
					right_value_upcasted,
					"fdiv")
			);
		}

		if (is_type_unsigned(highest_precision)) {
			return new value(
				"__udiv",
				highest_precision, 
				m_builder.CreateUDiv(
					left_value_upcasted, 
					right_value_upcasted, 
					"udiv")
			);
		}

		return new value(
			"__div",
			highest_precision, 
			m_builder.CreateSDiv(
				left_value_upcasted,
				right_value_upcasted,
				"div")
		);
	}

	value* codegen_visitor::visit_operator_modulo_node(operator_modulo_node& node) {
		const value* left = node.left->accept(*this);
		const value* right = node.right->accept(*this);

		const type highest_precision = get_highest_precision_type(left->get_type(), right->get_type());
		llvm::Value* left_value_upcasted = cast_value(left, highest_precision, node.get_declaration_line_index());
		llvm::Value* right_value_upcasted = cast_value(right, highest_precision, node.get_declaration_line_index());

		if (is_type_floating_point(highest_precision)) {
			return new value(
				"__frem",
				highest_precision, 
				m_builder.CreateFRem(
					left_value_upcasted, 
					right_value_upcasted, 
					"frem")
			);
		}

		if (is_type_unsigned(highest_precision)) {
			return new value(
				"__urem",
				highest_precision,
				m_builder.CreateURem(
					left_value_upcasted,
					right_value_upcasted, 
					"urem")
			);
		}

		return new value(
			"__rem",
			highest_precision,
			m_builder.CreateSRem(
				left_value_upcasted, 
				right_value_upcasted,
				"rem")
		);
	}
	
	bool codegen_visitor::has_main_entry_point() const {
		return m_functions.at("main") != nullptr;
	}

	value* codegen_visitor::get_declaration_value(const declaration_node& node) {
		// evaluate the expression to get the initial value
		value* assigned_value;

		if (node.get_expression()) {
			// evaluate the assigned value
			assigned_value = node.get_expression()->accept(*this);
		}
		else {
			// declared without an assigned value, set to 0
			llvm::Type* value_type = type_to_llvm_type(node.get_declaration_type(), m_context);
			assigned_value = new value(node.get_name(), node.get_declaration_type(), llvm::Constant::getNullValue(value_type));
		}

		return assigned_value;
	}

	llvm::Value* codegen_visitor::cast_value(const value* source_value, type target_type, u64 line_index) {
		// both types are the same 
		if(source_value->get_type() == target_type) {
			return source_value->get_value();
		}

		// cast function call
		if (source_value->get_type() == type::function_call) {
			// use the function return type as its type
			const type function_return_type = m_functions[source_value->get_name()]->get_return_type();

			// both types are the same 
			if(function_return_type == target_type) {
				return source_value->get_value();
			}

			compilation_logger::emit_function_return_type_cast_warning(line_index, function_return_type, target_type);

			llvm::Value* function_call_result = source_value->get_value();
			llvm::Type* target_llvm_type = type_to_llvm_type(target_type, function_call_result->getContext());

			// perform the cast op
			if (is_type_floating_point(function_return_type)) {
				if (is_type_integral(target_type)) {
					return m_builder.CreateFPToSI(function_call_result, target_llvm_type, "fptosi");
				}

				if (is_type_floating_point(target_type)) {
					return m_builder.CreateFPCast(function_call_result, target_llvm_type, "fpcast");
				}
			}
			else if (is_type_integral(function_return_type)) {
				if (is_type_floating_point(target_type)) {
					return m_builder.CreateSIToFP(function_call_result, target_llvm_type, "sitofp");
				}

				if (is_type_integral(target_type)) {
					return m_builder.CreateIntCast(function_call_result, target_llvm_type, is_type_signed(target_type), "intcast");
				}
			}
		}

		compilation_logger::emit_cast_warning(line_index, source_value->get_type(), target_type);

		// get the LLVM value and type for source and target
		llvm::Value* source_llvm_value = source_value->get_value();
		llvm::Type* target_llvm_type = type_to_llvm_type(target_type, source_llvm_value->getContext());

		// floating-point to integer
		if (is_type_floating_point(source_value->get_type()) && is_type_integral(target_type)) {
			return m_builder.CreateFPToSI(source_llvm_value, target_llvm_type);
		}

		// integer to floating-point
		if (is_type_integral(source_value->get_type()) && is_type_floating_point(target_type)) {
			return m_builder.CreateSIToFP(source_llvm_value, target_llvm_type);
		}

		// other cases
		if (get_type_bit_width(source_value->get_type()) < get_type_bit_width(target_type)) {
			// perform upcast
			if (is_type_unsigned(source_value->get_type())) {
				return m_builder.CreateZExt(source_llvm_value, target_llvm_type, "zext");
			}

			return m_builder.CreateSExt(source_llvm_value, target_llvm_type, "sext");
		}

		// downcast
		return m_builder.CreateTrunc(source_llvm_value, target_llvm_type, "trunc");
	}
}