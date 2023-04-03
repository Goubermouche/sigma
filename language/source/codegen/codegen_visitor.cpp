#include "codegen_visitor.h"

#include "abstract_syntax_tree/keywords/assignment_node.h"
#include "abstract_syntax_tree/keywords/declaration_node.h"
#include "abstract_syntax_tree/keywords/function_call_node.h"
#include "abstract_syntax_tree/variables/variable_node.h"
#include "abstract_syntax_tree/keywords/function_node.h"

// keywords
#include "abstract_syntax_tree/keywords/types/keyword_i8_node.h"
#include "abstract_syntax_tree/keywords/types/keyword_i16_node.h"
#include "abstract_syntax_tree/keywords/types/keyword_i32_node.h"
#include "abstract_syntax_tree/keywords/types/keyword_i64_node.h"

// operators
#include "abstract_syntax_tree/keywords/function_call_node.h"
#include "abstract_syntax_tree/operators/operator_addition_node.h"
#include "abstract_syntax_tree/operators/operator_subtraction_node.h"
#include "abstract_syntax_tree/operators/operator_multiplication_node.h"
#include "abstract_syntax_tree/operators/operator_division_node.h"
#include "abstract_syntax_tree/operators/operator_modulo_node.h"

#include <llvm/IR/Verifier.h>

namespace channel {
	codegen_visitor::codegen_visitor() : m_builder(m_context) {
		m_module = std::make_unique<llvm::Module>("channel", m_context);
	}

	void codegen_visitor::print_code() const {
		std::cout << "-----------------------------\n";
		m_module->print(llvm::outs(), nullptr);
	}

	void codegen_visitor::verify() const {
		std::cout << "-----------------------------\n";
		if(!llvm::verifyModule(*m_module, &llvm::outs())) {
			std::cout << "all checks passed\n";
		}
	}

	llvm::Value* codegen_visitor::visit_assignment_node(assignment_node& node) {
		// local variable
		if(llvm::Value* local_variable = m_named_values[node.get_name()]) {
			// evaluate the expression on the right-hand side of the assignment
			llvm::Value* new_value = node.get_expression()->accept(*this);
			// store the value in the memory location of the variable
			m_builder.CreateStore(new_value, local_variable);
			return new_value;
		}

		// global variable
		// look up the global variable in the namedGlobalValues map
		llvm::Value* pointer_to_global_variable = m_named_global_values[node.get_name()];
		ASSERT(pointer_to_global_variable, "[codegen]: variable not found (" + node.get_name() + ")");

		// evaluate the expression on the right-hand side of the assignment
		llvm::Value* new_value = node.get_expression()->accept(*this);

		// store the new value in the memory location of the global variable
		m_builder.CreateStore(new_value, pointer_to_global_variable);
		return new_value;
	}

	llvm::Value* codegen_visitor::visit_declaration_node(declaration_node& node) {
		// evaluate the expression to get the initial value
		llvm::Value* initial_value;
		// assume i32 type for now
		// todo: generalize
		llvm::Type* var_type = llvm::Type::getInt32Ty(m_context);

		if (node.get_expression()) {
			// evaluate the expression to get the initial value
			initial_value = node.get_expression()->accept(*this);
		}
		else {
			// use a default value for unassigned variables
			initial_value = llvm::ConstantInt::get(var_type, 0);
		}

		// create a global variable
		if(node.is_global()) {
			// linkage: https://stackoverflow.com/questions/28610783/are-global-variables-extern-by-default-or-is-it-equivalent-to-declaring-variable
			llvm::GlobalVariable* global_variable;
			if (auto* const_init_value = llvm::dyn_cast<llvm::Constant>(initial_value)) {
				// constant
				global_variable = new llvm::GlobalVariable(*m_module,
					initial_value->getType(),
					false,
					llvm::GlobalValue::InternalLinkage,
					const_init_value,
					node.get_name()
				);
			}
			else {
				// not constant
				global_variable = new llvm::GlobalVariable(*m_module,
					initial_value->getType(),
					false,
					llvm::GlobalValue::ExternalLinkage,
					llvm::Constant::getNullValue(initial_value->getType()), // default initializer
					node.get_name()
				);

				// store the non-constant initial value in the global variable
				m_builder.CreateStore(initial_value, global_variable);
			}

			// add the variable to the m_named_global_values map
			m_named_global_values[node.get_name()] = global_variable;
			return global_variable;
		}

		// create a local variable
		ASSERT(m_builder.GetInsertBlock(), "[codegen]: invalid insert block");
		const llvm::Function* current_function = m_builder.GetInsertBlock()->getParent();
		ASSERT(current_function, "[codegen]: invalid function");

		// store the initial value in the memory
		llvm::AllocaInst* alloca = m_builder.CreateAlloca(var_type, nullptr, node.get_name());
		m_builder.CreateStore(initial_value, alloca);
		m_named_values[node.get_name()] = alloca;

		return initial_value;
	}

	llvm::Value* codegen_visitor::visit_function_call_node(function_call_node& node) {
		llvm::Function* function = m_module->getFunction(node.get_name());
		ASSERT(function, "[codegen]: function not found (" + node.get_name() + ")");

		// generate code for each argument expression
		std::vector<llvm::Value*> argValues;
		for (channel::node* argument : node.get_arguments()) {
			argument->accept(*this);
			argValues.push_back(m_builder.GetInsertBlock()->getParent()->back().getTerminator()->getOperand(0));
		}

		return m_builder.CreateRet(m_builder.CreateCall(function, argValues, "call"));
	}

	llvm::Value* codegen_visitor::visit_variable_node(variable_node& node) {
		// local variable
		if (llvm::Value* variable_value = m_named_values[node.get_name()]) {
			// load the value from the memory location
			const llvm::AllocaInst* alloca = llvm::dyn_cast<llvm::AllocaInst>(variable_value);
			return m_builder.CreateLoad(alloca->getAllocatedType(), variable_value, node.get_name());
		}

		// global variable
		llvm::Value* pointer_to_global_variable = m_named_global_values[node.get_name()];
		ASSERT(pointer_to_global_variable, "[codegen]: variable not found (" + node.get_name() + ")");
		const llvm::GlobalValue* global_variable_value = llvm::dyn_cast<llvm::GlobalValue>(pointer_to_global_variable);
		return m_builder.CreateLoad(global_variable_value->getValueType(), pointer_to_global_variable, node.get_name());
	}

	llvm::Value* codegen_visitor::visit_function_node(function_node& node) {
		// assume 'int' return type for simplicity
		// todo: generalize
		llvm::Type* return_type = llvm::Type::getInt32Ty(m_context);
		llvm::FunctionType* function_type = llvm::FunctionType::get(return_type, false);
		llvm::Function* function = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, node.get_name(), m_module.get());
		llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(m_context, "entry", function);

		m_builder.SetInsertPoint(entryBlock);

		for (const auto& statement : node.get_statements()) {
			statement->accept(*this);
		}

		// add a return statement if the function does not have one
		if (!entryBlock->getTerminator()) {
			if (return_type->isVoidTy()) {
				m_builder.CreateRetVoid();
			}
			else {
				// assume 'int' return type
				// todo: generalize
				m_builder.CreateRet(llvm::ConstantInt::get(m_context, llvm::APInt(32, 0)));
			}
		}

		return function;
	}

	llvm::Value* codegen_visitor::visit_keyword_i8_node(keyword_i8_node& node) {
		return llvm::ConstantInt::get(m_context, llvm::APInt(8, node.value));
	}

	llvm::Value* codegen_visitor::visit_keyword_i16_node(keyword_i16_node& node) {
		return llvm::ConstantInt::get(m_context, llvm::APInt(16, node.value));
	}

	llvm::Value* codegen_visitor::visit_keyword_i32_node(keyword_i32_node& node) {
		return llvm::ConstantInt::get(m_context, llvm::APInt(32, node.get_value()));
	}

	llvm::Value* codegen_visitor::visit_keyword_i64_node(keyword_i64_node& node) {
		return llvm::ConstantInt::get(m_context, llvm::APInt(64, node.value));
	}

	llvm::Value* codegen_visitor::visit_operator_addition_node(operator_addition_node& node) {
		llvm::Value* left = node.left->accept(*this);
		llvm::Value* right = node.right->accept(*this);
		return m_builder.CreateAdd(left, right, "add");
	}

	llvm::Value* codegen_visitor::visit_operator_subtraction_node(operator_subtraction_node& node) {
		llvm::Value* left = node.left->accept(*this);
		llvm::Value* right = node.right->accept(*this);
		return m_builder.CreateSub(left, right, "sub");
	}

	llvm::Value* codegen_visitor::visit_operator_multiplication_node(operator_multiplication_node& node) {
		llvm::Value* left = node.left->accept(*this);
		llvm::Value* right = node.right->accept(*this);
		return m_builder.CreateMul(left, right, "mul");
	}

	llvm::Value* codegen_visitor::visit_operator_division_node(operator_division_node& node) {
		llvm::Value* left = node.left->accept(*this);
		llvm::Value* right = node.right->accept(*this);
		return m_builder.CreateSDiv(left, right, "div");
	}

	llvm::Value* codegen_visitor::visit_operator_modulo_node(operator_modulo_node& node) {
		llvm::Value* left = node.left->accept(*this);
		llvm::Value* right = node.right->accept(*this);
		return m_builder.CreateSRem(left, right, "mod");
	}
}