#include "codegen_visitor.h"

#include "abstract_syntax_tree/keywords/assignment_node.h"
#include "abstract_syntax_tree/keywords/function_call_node.h"
#include "abstract_syntax_tree/keywords/function_node.h"

#include "abstract_syntax_tree/variables/variable_node.h"
#include "abstract_syntax_tree/variables/declaration/declaration_node.h"
#include "abstract_syntax_tree/variables/declaration/local_declaration_node.h"
#include "abstract_syntax_tree/variables/declaration/global_declaration_node.h"

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

#include "abstract_syntax_tree/keywords/return_node.h"

namespace channel {
	codegen_visitor::codegen_visitor(parser& parser)
		: m_scope(new scope(nullptr)), m_builder(m_context) {
		m_module = std::make_unique<llvm::Module>("channel", m_context);

		// walk the abstract syntax tree
		const std::vector<node*> abstract_syntax_tree = parser.parse();
		for (node* node : abstract_syntax_tree) {
			node->accept(*this);
		}

		initialize_global_variables();
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

	llvm::Value* codegen_visitor::visit_assignment_node(assignment_node& node) {
		// local variable
		if (llvm::Value* local_variable = m_scope->get_value(node.get_name())) {
			// evaluate the expression on the right-hand side of the assignment
			llvm::Value* new_value = node.get_expression()->accept(*this);
			// store the value in the memory location of the variable
			m_builder.CreateStore(new_value, local_variable);
			return new_value;
		}

		// global variable
		// look up the global variable in the m_global_named_values map
		llvm::Value* pointer_to_global_variable = m_global_named_values[node.get_name()];
		ASSERT(pointer_to_global_variable, "[codegen]: variable '" + node.get_name() + "' not found");

		// evaluate the expression on the right-hand side of the assignment
		llvm::Value* new_value = node.get_expression()->accept(*this);

		// store the new value in the memory location of the global variable
		m_builder.CreateStore(new_value, pointer_to_global_variable);
		return new_value;
	}

	llvm::Value* codegen_visitor::visit_function_call_node(function_call_node& node) {
		llvm::Function* function = m_module->getFunction(node.get_name());
		ASSERT(function, "[codegen]: function '" + node.get_name() + ")");

		// generate code for each argument expression
		std::vector<llvm::Value*> argument_values;
		for (channel::node* argument : node.get_arguments()) {
			argument->accept(*this);
			argument_values.push_back(m_builder.GetInsertBlock()->getParent()->back().getTerminator()->getOperand(0));
		}

		return m_builder.CreateCall(function, argument_values, "call");
	}

	llvm::Value* codegen_visitor::visit_variable_node(variable_node& node) {
		// local variable
		if (llvm::Value* variable_value = m_scope->get_value(node.get_name())) {
			// load the value from the memory location
			const llvm::AllocaInst* alloca = llvm::dyn_cast<llvm::AllocaInst>(variable_value);
			return m_builder.CreateLoad(alloca->getAllocatedType(), variable_value, node.get_name());
		}

		// global variable
		llvm::Value* pointer_to_global_variable = m_global_named_values[node.get_name()];
		ASSERT(pointer_to_global_variable, "[codegen]: variable '" + node.get_name() + "' not found");
		const llvm::GlobalValue* global_variable_value = llvm::dyn_cast<llvm::GlobalValue>(pointer_to_global_variable);
		return m_builder.CreateLoad(global_variable_value->getValueType(), pointer_to_global_variable, node.get_name());
	}

	llvm::Value* codegen_visitor::visit_function_node(function_node& node) {
		// assume 'int' return type for the sake of simplicity
		// todo: generalize
		llvm::Type* return_type = llvm::Type::getInt32Ty(m_context);
		llvm::FunctionType* function_type = llvm::FunctionType::get(return_type, false);
		llvm::Function* function = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, node.get_name(), m_module.get());
		llvm::BasicBlock* entry_block = llvm::BasicBlock::Create(m_context, "entry", function);
		m_builder.SetInsertPoint(entry_block);

		// create a new nested scope for the function body
		scope* prev_scope = m_scope;
		m_scope = new scope(prev_scope);

		// if the function is 'main', call the '_global_init' function
		if (node.get_name() == "main") {
			m_main_entry_point = function;
		}

		// accept all statements inside the function
		for (const auto& statement : node.get_statements()) {
			statement->accept(*this);
		}

		// restore the previous scope
		m_scope = prev_scope;

		// add a return statement if the function does not have one
		if (!entry_block->getTerminator()) {
			if (return_type->isVoidTy()) {
				m_builder.CreateRetVoid();
			}
			else {
				// assume 'int' return type
				// todo: generalize
				m_builder.CreateRet(llvm::ConstantInt::get(m_context, llvm::APInt(32, 0, true)));
			}
		}

		return function;
	}

	llvm::Value* codegen_visitor::visit_return_node(return_node& node) {
		// evaluate the expression of the return statement
		llvm::Value* return_value = node.get_expression()->accept(*this);
		// generate the LLVM return instruction with the evaluated expression
		m_builder.CreateRet(return_value);
		// return the value of the expression
		return return_value;
	}

	llvm::Value* codegen_visitor::visit_local_declaration_node(local_declaration_node& node) {
		// evaluate the assigned value, if there is one
		llvm::Value* initial_value = get_declaration_value(node);

		// create a local variable
		ASSERT(m_builder.GetInsertBlock(), "[codegen]: invalid insert block");
		const llvm::Function* current_function = m_builder.GetInsertBlock()->getParent();
		ASSERT(current_function, "[codegen]: invalid function");

		// store the initial value in the memory
		llvm::AllocaInst* alloca = m_builder.CreateAlloca(initial_value->getType(), nullptr, node.get_name());
		m_builder.CreateStore(initial_value, alloca);

		// add the variable to the current scope
		ASSERT(!m_global_named_values[node.get_name()], "[codegen]: local variable '" + node.get_name() + "' has alredy been defined in the global scope");
		const auto insertion_result = m_scope->add_named_value(node.get_name(), alloca);
		ASSERT(insertion_result.second, "[codegen]: local variable '" + node.get_name() + "' has already been defined before");
		return initial_value;
	}

	llvm::Value* codegen_visitor::visit_global_declaration_node(global_declaration_node& node) {
		// only generate the global variable if the main entry point hasn't been
		// declared yet
		if (m_main_entry_point) {
			return nullptr;
		}

		// start creating the init function
		const std::string init_func_name = "__global_init_" + node.get_name();
		llvm::FunctionType* init_func_type = llvm::FunctionType::get(llvm::Type::getVoidTy(m_context), false);
		llvm::Function* init_func = llvm::Function::Create(init_func_type, llvm::Function::InternalLinkage, init_func_name, m_module.get());
		llvm::BasicBlock* init_func_entry = llvm::BasicBlock::Create(m_context, "entry", init_func);
		m_builder.SetInsertPoint(init_func_entry); // write to the init function

		// evaluate the assigned value, if there is one
		llvm::Value* initial_value = get_declaration_value(node);
		llvm::Type* value_type = initial_value->getType();

		// todo: const evaluation of global variables
		// constant-evaluated
		//if(auto* constant_initial_value = llvm::dyn_cast<llvm::Constant>(initial_value)) {
		//	
		//}
		//else {
		//	
		//}

		// create a global variable
		auto* global_variable = new llvm::GlobalVariable(*m_module,
			value_type,
			false,
			llvm::GlobalValue::ExternalLinkage,
			llvm::Constant::getNullValue(value_type), // default initializer
			node.get_name()
		);

		// add the variable to the m_global_named_values map
		const auto insertion_result = m_global_named_values.insert({ node.get_name(), global_variable });
		ASSERT(insertion_result.second, "[codegen]: global variable '" + node.get_name() + "' has already been defined before");

		m_builder.CreateStore(initial_value, global_variable);
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

	llvm::Value* codegen_visitor::visit_keyword_i8_node(keyword_i8_node& node) {
		return llvm::ConstantInt::get(m_context, llvm::APInt(8, node.value, true));
	}

	llvm::Value* codegen_visitor::visit_keyword_i16_node(keyword_i16_node& node) {
		return llvm::ConstantInt::get(m_context, llvm::APInt(16, node.value, true));
	}

	llvm::Value* codegen_visitor::visit_keyword_i32_node(keyword_i32_node& node) {
		return llvm::ConstantInt::get(m_context, llvm::APInt(32, node.get_value(), true));
	}

	llvm::Value* codegen_visitor::visit_keyword_i64_node(keyword_i64_node& node) {
		return llvm::ConstantInt::get(m_context, llvm::APInt(64, node.value, true));
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

	bool codegen_visitor::has_main_entry_point() const {
		return m_module->getFunction("main") != nullptr;
	}

	llvm::Value* codegen_visitor::get_declaration_value(const declaration_node& node) {
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
			// todo: generalize
			initial_value = llvm::ConstantInt::get(var_type, 0, true);
		}

		return initial_value;
	}
}
