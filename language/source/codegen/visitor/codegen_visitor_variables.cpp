#include "codegen_visitor.h"

#include "../abstract_syntax_tree/variables/assignment_node.h"
#include "../abstract_syntax_tree/variables/variable_node.h"
#include "../abstract_syntax_tree/variables/declaration/local_declaration_node.h"
#include "../abstract_syntax_tree/variables/declaration/global_declaration_node.h"

namespace channel {
	bool codegen_visitor::visit_assignment_node(assignment_node& node, value*& out_value) {
		// local variable
		if (const value* local_variable = m_scope->get_named_value(node.get_name())) {
			// evaluate the expression on the right-hand side of the assignment
			if (!node.get_expression()->accept(*this, out_value)) {
				return false;
			}

			// upcast the new_value if necessary
			out_value->set_value(cast_value(out_value, local_variable->get_type(), node.get_declaration_line_index()));

			// store the value in the memory location of the variable
			m_builder.CreateStore(out_value->get_value(), local_variable->get_value());
			return true;
		}

		// global variable
		// look up the global variable in the m_global_named_values map
		const value* pointer_to_global_variable = m_global_named_values[node.get_name()];
		if (!pointer_to_global_variable) {
			compilation_logger::emit_variable_not_found_error(node.get_declaration_line_index(), node.get_name());
			return false;
		}

		// evaluate the expression on the right-hand side of the assignment
		if (!node.get_expression()->accept(*this, out_value)) {
			return false;
		}

		// upcast the new_value if necessary
		out_value->set_value(cast_value(out_value, pointer_to_global_variable->get_type(), node.get_declaration_line_index()));

		// store the new value in the memory location of the global variable
		m_builder.CreateStore(out_value->get_value(), pointer_to_global_variable->get_value());
		return true;
	}

	bool codegen_visitor::visit_variable_node(variable_node& node, value*& out_value) {
		// local variable
		if (const value* variable_value = m_scope->get_named_value(node.get_name())) {
			// load the value from the memory location
			const llvm::AllocaInst* alloca = llvm::dyn_cast<llvm::AllocaInst>(variable_value->get_value());

			llvm::Value* load = m_builder.CreateLoad(
				alloca->getAllocatedType(),
				variable_value->get_value(),
				node.get_name()
			);

			out_value = new value(node.get_name(), variable_value->get_type(), load);
			return true;
		}

		// global variable
		const value* pointer_to_global_variable = m_global_named_values[node.get_name()];

		if (!pointer_to_global_variable) {
			compilation_logger::emit_variable_not_found_error(node.get_declaration_line_index(), node.get_name());
			return false;
		}

		const llvm::GlobalValue* global_variable_value = llvm::dyn_cast<llvm::GlobalValue>(pointer_to_global_variable->get_value());

		llvm::Value* load = m_builder.CreateLoad(
			global_variable_value->getValueType(),
			pointer_to_global_variable->get_value(),
			node.get_name()
		);

		out_value = new value(node.get_name(), pointer_to_global_variable->get_type(), load);
		return true;
	}

	bool codegen_visitor::visit_local_declaration_node(local_declaration_node& node, value*& out_value) {
		// evaluate the assigned value, if there is one
		if (!get_declaration_value(node, out_value)) {
			return false;
		}

		// const type highest_precision = get_highest_precision_type(node.get_declaration_type(), assigned_value->get_type());
		llvm::Value* upcasted_assigned_value = cast_value(out_value, node.get_declaration_type(), node.get_declaration_line_index());

		// store the initial value in the memory
		llvm::AllocaInst* alloca = m_builder.CreateAlloca(
			type_to_llvm_type(node.get_declaration_type(), m_context),
			nullptr,
			node.get_name()
		);

		m_builder.CreateStore(upcasted_assigned_value, alloca);

		// add the variable to the current scope
		if (m_global_named_values[node.get_name()]) {
			compilation_logger::emit_local_variable_already_defined_in_global_scope_error(node.get_declaration_line_index(), node.get_name());
			return false;
		}

		const auto insertion_result = m_scope->add_named_value(node.get_name(), new value(node.get_name(), node.get_declaration_type(), alloca));
		if (!insertion_result.second) {
			compilation_logger::emit_local_variable_already_defined_error(node.get_declaration_line_index(), node.get_name());
			return false;
		}

		return true;
	}

	bool codegen_visitor::visit_global_declaration_node(global_declaration_node& node, value*& out_value) {
		// start creating the init function
		const std::string init_func_name = "__global_init_" + node.get_name();
		llvm::FunctionType* init_func_type = llvm::FunctionType::get(llvm::Type::getVoidTy(m_context), false);
		llvm::Function* init_func = llvm::Function::Create(init_func_type, llvm::Function::InternalLinkage, init_func_name, m_module.get());
		llvm::BasicBlock* init_func_entry = llvm::BasicBlock::Create(m_context, "entry", init_func);
		m_builder.SetInsertPoint(init_func_entry); // write to the init function

		// evaluate the assigned value, if there is one
		value* assigned_value;
		if (!get_declaration_value(node, assigned_value)) {
			return false;
		}

		// const type highest_precision = get_highest_precision_type(node.get_declaration_type(), initial_value->get_type());
		llvm::Value* upcasted_initial_value = cast_value(assigned_value, node.get_declaration_type(), node.get_declaration_line_index());

		// create a global variable
		out_value = new value(
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
		const auto insertion_result = m_global_named_values.insert({ node.get_name(),  out_value });
		if (!insertion_result.second) {
			compilation_logger::emit_global_variable_already_defined_error(node.get_declaration_line_index(), node.get_name());
			return false;
		}

		m_builder.CreateStore(upcasted_initial_value, out_value->get_value());
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
		return true;
	}

	bool codegen_visitor::get_declaration_value(const declaration_node& node, value*& out_value) {
		// evaluate the expression to get the initial value

		if (node.get_expression()) {
			// evaluate the assigned value
			return node.get_expression()->accept(*this, out_value);
		}

		// declared without an assigned value, set to 0
		llvm::Type* value_type = type_to_llvm_type(node.get_declaration_type(), m_context);
		out_value = new value(node.get_name(), node.get_declaration_type(), llvm::Constant::getNullValue(value_type));
		return true;
	}
}