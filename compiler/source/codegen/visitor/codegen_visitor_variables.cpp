#include "codegen_visitor.h"

#include "../abstract_syntax_tree/variables/assignment_node.h"
#include "../abstract_syntax_tree/variables/access_node.h"
#include "../abstract_syntax_tree/variables/variable_node.h"
#include "../abstract_syntax_tree/variables/array/array_allocation_node.h"
#include "../abstract_syntax_tree/variables/array/array_assignment_node.h"
#include "../abstract_syntax_tree/variables/array/array_access_node.h"
#include "../abstract_syntax_tree/variables/declaration/local_declaration_node.h"
#include "../abstract_syntax_tree/variables/declaration/global_declaration_node.h"

namespace channel {
	bool codegen_visitor::visit_assignment_node(assignment_node& node, value*& out_value) {
		// evaluate the expression on the right-hand side of the assignment
		if (!node.get_expression_node()->accept(*this, out_value)) {
			return false;
		}

		// assignment to a local variable
		// look up the local variable in the active scope
		value* variable;
		if (!get_named_value(variable, node.get_variable_identifier())) {
			compilation_logger::emit_variable_not_found_error(node.get_declaration_line_number(), node.get_variable_identifier());
			return false;
		}

		llvm::Value* out_cast;
		if(!cast_value(out_cast, out_value, variable->get_type(), node.get_declaration_line_number())) {
			return false;
		}

		out_value->set_value(out_cast);
		m_builder.CreateStore(out_value->get_value(), variable->get_value());
		return true;
	}

	bool codegen_visitor::visit_access_node(access_node& node, value*& out_value) {
		// load a local variable
		// look up the local variable in the active scope
		if (const value* variable_value = m_scope->get_named_value(node.get_variable_identifier())) {
			// load the value from the memory location
			const llvm::AllocaInst* alloca = llvm::dyn_cast<llvm::AllocaInst>(variable_value->get_value());
			llvm::Value* load = m_builder.CreateLoad(
				alloca->getAllocatedType(),
				variable_value->get_value(),
				node.get_variable_identifier()
			);

			// return the load instruction as a value
			out_value = new value(node.get_variable_identifier(), variable_value->get_type(), load);
			return true;
		}

		// we haven't found a local variable, check if we're loading a global one
		// load a global variable
		// look up the global variable in the m_global_named_values map
		const value* global_variable = m_global_named_values[node.get_variable_identifier()];
		// check if the global variable exists
		if (!global_variable) {
			compilation_logger::emit_variable_not_found_error(node.get_declaration_line_number(), node.get_variable_identifier());
			return false;
		}

		// get the value from the global variable pointer
		const llvm::GlobalValue* global_variable_value = llvm::dyn_cast<llvm::GlobalValue>(global_variable->get_value());
		// load the value from the memory location
		llvm::Value* load = m_builder.CreateLoad(
			global_variable_value->getValueType(),
			global_variable->get_value(),
			node.get_variable_identifier()
		);

		// return the load instruction as a value
		out_value = new value(node.get_variable_identifier(), global_variable->get_type(), load);
		return true;
	}

	bool codegen_visitor::visit_local_declaration_node(local_declaration_node& node, value*& out_value) {
		// evaluate the assigned value, if there is one
		if (!get_declaration_value(node, out_value)) {
			return false;
		}

		// cast the right-hand side operator to the assigned type
		llvm::Value* cast_assigned_value;
		if (!cast_value(cast_assigned_value, out_value, node.get_declaration_type(), node.get_declaration_line_number())) {
			return false;
		}

		// store the initial value
		llvm::AllocaInst* alloca = m_builder.CreateAlloca(
			node.get_declaration_type().get_llvm_type(m_context),
			nullptr,
			node.get_declaration_identifier()
		);

		m_builder.CreateStore(cast_assigned_value, alloca);

		// check if the variable already exists as a global
		if (m_global_named_values[node.get_declaration_identifier()]) {
			compilation_logger::emit_local_variable_already_defined_in_global_scope_error(node.get_declaration_line_number(), node.get_declaration_identifier());
			return false;
		}

		// add the variable to the active scope
		const auto insertion_result = m_scope->add_named_value(node.get_declaration_identifier(), new value(node.get_declaration_identifier(), node.get_declaration_type(), alloca));
		// check if the active scope already contains the variable
		if (!insertion_result.second) {
			compilation_logger::emit_local_variable_already_defined_error(node.get_declaration_line_number(), node.get_declaration_identifier());
			return false;
		}

		return true;
	}

	bool codegen_visitor::visit_global_declaration_node(global_declaration_node& node, value*& out_value) {
		// start creating the init function for our global ctor
		const std::string init_func_name = "__global_init_" + node.get_declaration_identifier();
		llvm::FunctionType* init_func_type = llvm::FunctionType::get(llvm::Type::getVoidTy(m_context), false);
		llvm::Function* init_func = llvm::Function::Create(init_func_type, llvm::Function::InternalLinkage, init_func_name, m_module.get());
		llvm::BasicBlock* init_func_entry = llvm::BasicBlock::Create(m_context, "entry", init_func);
		m_builder.SetInsertPoint(init_func_entry); // write to the init function

		// evaluate the assigned value, if there is one
		value* assigned_value;
		if (!get_declaration_value(node, assigned_value)) {
			return false;
		}

		// cast the right-hand side operator to the assigned type
		llvm::Value* cast_assigned_value;
		if(!cast_value(cast_assigned_value, assigned_value, node.get_declaration_type(), node.get_declaration_line_number())) {
			return false;
		}

		// create a global variable
		out_value = new value(
			node.get_declaration_identifier(),
			node.get_declaration_type(),
			new llvm::GlobalVariable(*m_module,
				node.get_declaration_type().get_llvm_type(m_context),
				false,
				llvm::GlobalValue::ExternalLinkage,
				llvm::Constant::getNullValue(node.get_declaration_type().get_llvm_type(m_context)), // default initializer
				node.get_declaration_identifier()
			)
		);

		// add the variable to the m_global_named_values map
		const auto insertion_result = m_global_named_values.insert({ node.get_declaration_identifier(),  out_value });
		// check if a global with the same name already exists
		if (!insertion_result.second) {
			compilation_logger::emit_global_variable_already_defined_error(node.get_declaration_line_number(), node.get_declaration_identifier());
			return false;
		}

		m_builder.CreateStore(cast_assigned_value, out_value->get_value());
		m_builder.CreateRetVoid();

		// create a new constructor with the given priority
		llvm::ConstantInt* priority = llvm::ConstantInt::get(llvm::Type::getInt32Ty(m_context), m_global_initialization_priority++);
		llvm::Constant* initializer_cast = llvm::ConstantExpr::getBitCast(init_func, llvm::Type::getInt8PtrTy(m_context));
		llvm::Constant* new_ctor = llvm::ConstantStruct::get(CTOR_STRUCT_TYPE, {
			priority,
			initializer_cast,
			llvm::Constant::getNullValue(llvm::Type::getInt8PtrTy(m_context))
		});

		// push the constructor to the ctor list
		m_global_ctors.push_back(new_ctor);
		return true;
	}

	bool codegen_visitor::visit_allocation_node(array_allocation_node& node, value*& out_value) {
		// get the count of allocated elements
		value* element_count;
		if (!node.get_array_element_count_node()->accept(*this, element_count)) {
			return false;
		}

		// cast the element count node to i64
		llvm::Value* element_count_cast;
		if (!cast_value(element_count_cast, element_count, type(type::base::i64, 0), node.get_array_element_count_node()->get_declaration_line_number())) {
			return false;
		}

		const llvm::FunctionCallee malloc_func = m_module->getOrInsertFunction(
			"malloc",
			llvm::FunctionType::get(
				llvm::Type::getInt8PtrTy(m_context),
				llvm::Type::getInt64Ty(m_context),
				false
			)
		);

		// calculate the total size
		llvm::Type* element_type = node.get_array_element_type().get_llvm_type(m_context);
		llvm::Value* element_size = llvm::ConstantInt::get(m_context, llvm::APInt(64, (node.get_array_element_type().get_bit_width() + 7) / 8));
		llvm::Value* total_size = m_builder.CreateMul(element_count_cast, element_size);

		llvm::Value* allocated_ptr = m_builder.CreateCall(malloc_func, total_size);

		// cast the result to the correct pointer type
		llvm::Value* typed_ptr = m_builder.CreateBitCast(allocated_ptr, llvm::PointerType::getUnqual(element_type));

		out_value = new value("__alloca", node.get_array_element_type().get_pointer_type(), typed_ptr);
		return true;
	}

	bool codegen_visitor::visit_array_access_node(array_access_node& node, value*& out_value) {
		const std::vector<channel::node*>& index_nodes = node.get_array_element_index_nodes();
		value* array_ptr;

		// evaluate the array base expression
		if (!node.get_array_base_node()->accept(*this, array_ptr)) {
			return false;
		}

		// traverse the array indexes
		type current_type = array_ptr->get_type();
		llvm::Value* current_ptr = array_ptr->get_value();

		for (size_t i = 0; i < index_nodes.size(); ++i) {
			value* index_value;

			if (!index_nodes[i]->accept(*this, index_value)) {
				return false;
			}

			// cast the index value to i64
			llvm::Value* index_value_cast;
			if (!cast_value(index_value_cast, index_value, type(type::base::i64, 0), node.get_declaration_line_number())) {
				return false;
			}

			// load the actual pointer value
			current_ptr = m_builder.CreateLoad(current_type.get_llvm_type(m_context), current_ptr);

			// get the next level pointer
			// current_ptr = m_builder.CreateGEP(current_type.get_element_type().get_llvm_type(m_context), current_ptr, index_value_cast);
			current_ptr = m_builder.CreateInBoundsGEP(current_type.get_element_type().get_llvm_type(m_context), current_ptr, index_value_cast); // Changed to CreateInBoundsGEP

			// update the current_type for the next iteration
			if (i != index_nodes.size() - 1) {
				current_type = current_type.get_element_type();
			}
		}

		// load the value at the final element address
		llvm::Value* loaded_value = m_builder.CreateLoad(current_type.get_element_type().get_llvm_type(m_context), current_ptr);

		out_value = new value("array_element", current_type.get_element_type(), loaded_value);
		return true;
	}

	bool codegen_visitor::visit_array_assignment_node(array_assignment_node& node, value*& out_value) {
		const std::vector<channel::node*>& index_nodes = node.get_array_element_index_nodes();
		value* array_ptr;

		// evaluate the array base expression
		if (!node.get_array_base_node()->accept(*this, array_ptr)) {
			return false;
		}

		// traverse the array indexes
		type current_type = array_ptr->get_type();
		llvm::Value* current_ptr = array_ptr->get_value();

		for (size_t i = 0; i < index_nodes.size(); ++i) {
			value* index_value;

			if (!index_nodes[i]->accept(*this, index_value)) {
				return false;
			}

			// cast the index value to i64
			llvm::Value* index_value_cast;
			if (!cast_value(index_value_cast, index_value, type(type::base::i64, 0), node.get_declaration_line_number())) {
				return false;
			}

			// load the actual pointer value
			current_ptr = m_builder.CreateLoad(current_type.get_llvm_type(m_context), current_ptr);

			// get the next level pointer
			// current_ptr = m_builder.CreateGEP(current_type.get_element_type().get_llvm_type(m_context), current_ptr, index_value_cast);
			current_ptr = m_builder.CreateInBoundsGEP(current_type.get_element_type().get_llvm_type(m_context), current_ptr, index_value_cast); // Changed to CreateInBoundsGEP

			// update the current_type for the next iteration
			if (i != index_nodes.size() - 1) {
				current_type = current_type.get_element_type();
			}
		}

		// evaluate the right-hand side expression
		value* expression_value;
		if (!node.get_expression_node()->accept(*this, expression_value)) {
			return false;
		}

		// get the final element type for the assignment
		const type final_element_type = current_type.get_element_type();

		// cast the expression value to the array element type
		llvm::Value* expression_llvm_value_cast;
		if (!cast_value(expression_llvm_value_cast, expression_value, final_element_type, node.get_declaration_line_number())) {
			return false;
		}

		expression_value = new value(expression_value->get_name(), final_element_type, expression_llvm_value_cast);

		// store the result of the right-hand side expression in the array
		m_builder.CreateStore(expression_value->get_value(), current_ptr);
		out_value = expression_value;
		return true;
	}

	bool codegen_visitor::visit_variable_node(variable_node& node, value*& out_value) {
		// find the variable value in our named values
		value* var_value;
		if (!get_named_value(var_value, node.get_variable_identifier())) {
			compilation_logger::emit_variable_not_found_error(node.get_declaration_line_number(), node.get_variable_identifier());
			return false;
		}

		// since the variable_node represents an address, we do not need to load it
		// just return the found value
		out_value = var_value;
		return true;
	}

	bool codegen_visitor::get_declaration_value(const declaration_node& node, value*& out_value) {
		// evaluate the expression to get the initial value
		if (channel::node* expression = node.get_expression_node()) {
			// evaluate the assigned value
			return expression->accept(*this, out_value);
		}

		// declared without an assigned value, set it to 0
		llvm::Type* value_type = node.get_declaration_type().get_llvm_type(m_context);
		out_value = new value(node.get_declaration_identifier(), node.get_declaration_type(), llvm::Constant::getNullValue(value_type));
		return true;
	}
}
