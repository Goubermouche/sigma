#include "codegen_visitor.h"

#include "codegen/abstract_syntax_tree/variables/assignment_node.h"
#include "codegen/abstract_syntax_tree/variables/variable_access_node.h"
#include "codegen/abstract_syntax_tree/variables/variable_node.h"
#include "codegen/abstract_syntax_tree/variables/array/array_allocation_node.h"
#include "codegen/abstract_syntax_tree/variables/array/array_assignment_node.h"
#include "codegen/abstract_syntax_tree/variables/array/array_access_node.h"
#include "codegen/abstract_syntax_tree/variables/declaration/local_declaration_node.h"
#include "codegen/abstract_syntax_tree/variables/declaration/global_declaration_node.h"

namespace channel {
	acceptation_result codegen_visitor::visit_assignment_node(
		assignment_node& node, 
		const codegen_context& context
	) {
		// assignment to a local variable
		// look up the local variable in the active scope
		acceptation_result variable_result = node.get_variable_node()->accept(*this, {});
		if(!variable_result.has_value()) {
			return variable_result;
		}

		// evaluate the expression on the right-hand side of the assignment
		acceptation_result expression_result = node.get_expression_node()->accept(*this, codegen_context(variable_result.value()->get_type()));
		if (!expression_result.has_value()) {
			return expression_result;
		}

		value_ptr expression_value = expression_result.value();
		llvm::Value* out_cast = cast_value(expression_value, variable_result.value()->get_type(), node.get_declared_position());

		expression_value->set_value(out_cast);
		m_builder.CreateStore(expression_value->get_value(), variable_result.value()->get_value());
		return expression_value;
	}

	acceptation_result codegen_visitor::visit_variable_access_node(
		variable_access_node& node, 
		const codegen_context& context
	) {
		// load a local variable
		// look up the local variable in the active scope
		if (const value_ptr variable_value = m_scope->get_named_value(node.get_variable_identifier())) {
			// load the value from the memory location
			llvm::AllocaInst* alloca = llvm::dyn_cast<llvm::AllocaInst>(variable_value->get_value());
			llvm::Value* load = m_builder.CreateLoad(
				alloca->getAllocatedType(),
				variable_value->get_value()
			);

			// return the load instruction as a value
			value_ptr variable_load = std::make_shared<value>(node.get_variable_identifier(), variable_value->get_type(), load);
			variable_load->set_pointer(alloca);
			return variable_load;
		}

		// we haven't found a local variable, check if we're loading a global one
		// load a global variable
		// look up the global variable in the m_global_named_values map
		const value_ptr global_variable = m_global_named_values[node.get_variable_identifier()];
		// check if the global variable exists
		if (!global_variable) {
			return std::unexpected(error::emit<4003>(node.get_declared_position(), node.get_variable_identifier()));
		}

		// get the value from the global variable pointer
		const llvm::GlobalValue* global_variable_value = llvm::dyn_cast<llvm::GlobalValue>(global_variable->get_value());
		// load the value from the memory location
		llvm::Value* load = m_builder.CreateLoad(
			global_variable_value->getValueType(),
			global_variable->get_value()
		);

		// return the load instruction as a value
		return std::make_shared<value>(node.get_variable_identifier(), global_variable->get_type(), load);
	}

	acceptation_result codegen_visitor::visit_local_declaration_node(
		local_declaration_node& node, 
		const codegen_context& context
	) {
		llvm::BasicBlock* original_entry_block = m_builder.GetInsertBlock();
		llvm::Function* parent_function = original_entry_block->getParent();
		llvm::BasicBlock* function_entry_block = &*parent_function->begin();

		m_builder.SetInsertPoint(function_entry_block, function_entry_block->getFirstInsertionPt());

		// store the initial value
		llvm::AllocaInst* alloca = m_builder.CreateAlloca(
			node.get_declaration_type().get_llvm_type(m_context),
			nullptr
		);

		// check if the variable already exists as a global
		if (m_global_named_values[node.get_declaration_identifier()]) {
			return std::unexpected(error::emit<4004>(node.get_declared_position(), node.get_declaration_identifier()));
		}

		// add the variable to the active scope
		const auto insertion_result = m_scope->add_named_value(
			node.get_declaration_identifier(), 
			std::make_shared<value>(
				node.get_declaration_identifier(), 
				node.get_declaration_type(),
				alloca
			)
		);

		// check if the active scope already contains the variable
		if (!insertion_result.second) {
			return std::unexpected(error::emit<4005>(node.get_declared_position(), node.get_declaration_identifier()));
		}

		// assign the actual value
		m_builder.SetInsertPoint(original_entry_block);

		// evaluate the assigned value, if there is one
		acceptation_result declaration_value_result = get_declaration_value(node, codegen_context(node.get_declaration_type()));
		if (!declaration_value_result.has_value()) {
			return declaration_value_result;
		}

		value_ptr declaration_value = declaration_value_result.value();
		llvm::Value* cast_assigned_value = cast_value(declaration_value, node.get_declaration_type(), node.get_declared_position());

		m_builder.CreateStore(cast_assigned_value, alloca);
		declaration_value->set_pointer(alloca);
		m_builder.SetInsertPoint(original_entry_block);
		return declaration_value;
	}

	acceptation_result codegen_visitor::visit_global_declaration_node(
		global_declaration_node& node,
		const codegen_context& context
	) {
		// start creating the init function for our global ctor
		const std::string init_func_name = "__global_init_" + node.get_declaration_identifier();
		llvm::FunctionType* init_func_type = llvm::FunctionType::get(llvm::Type::getVoidTy(m_context), false);
		llvm::Function* init_func = llvm::Function::Create(init_func_type, llvm::Function::InternalLinkage, init_func_name, m_module.get());
		llvm::BasicBlock* init_func_entry = llvm::BasicBlock::Create(m_context, "", init_func);
		m_builder.SetInsertPoint(init_func_entry); // write to the init function

		// evaluate the assigned value, if there is one
		acceptation_result declaration_value_result = get_declaration_value(node, codegen_context(node.get_declaration_type()));
		if (!declaration_value_result.has_value()) {
			return declaration_value_result;
		}

		// cast the right-hand side operator to the assigned type
		llvm::Value* cast_assigned_value = cast_value(declaration_value_result.value(), node.get_declaration_type(), node.get_declared_position());

		// create a global variable
		value_ptr global_declaration = std::make_shared<value>(
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
		const auto insertion_result = m_global_named_values.insert({ node.get_declaration_identifier(),  global_declaration });
		// check if a global with the same name already exists
		if (!insertion_result.second) {
			return std::unexpected(error::emit<4006>(node.get_declared_position(), node.get_declaration_identifier()));
		}

		m_builder.CreateStore(cast_assigned_value, global_declaration->get_value());
		global_declaration->set_pointer(global_declaration->get_value());
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
		return global_declaration;
	}

	acceptation_result codegen_visitor::visit_allocation_node(
		array_allocation_node& node,
		const codegen_context& context
	) {
		// get the count of allocated elements
		acceptation_result element_count_result = node.get_array_element_count_node()->accept(*this, {});
		if (!element_count_result.has_value()) {
			return element_count_result;
		}

		// cast the element count node to u64
		llvm::Value* element_count_cast = cast_value(element_count_result.value(), type(type::base::u64, 0), node.get_array_element_count_node()->get_declared_position());

		const llvm::FunctionCallee malloc_func = m_function_registry.get_function("malloc")->get_function();

		// calculate the total size
		const type array_element_type = node.get_array_element_type();
		llvm::Type* element_type = array_element_type.get_llvm_type(m_context);
		llvm::Value* element_size = llvm::ConstantInt::get(m_context, llvm::APInt(64, (node.get_array_element_type().get_bit_width() + 7) / 8));
		llvm::Value* total_size = m_builder.CreateMul(element_count_cast, element_size);

		// check if we have a char*
		const bool is_char_pointer = array_element_type.get_base() == type::base::character && array_element_type.get_pointer_level() == 0;

		// add space for a null terminator if the element type is a character
		if (is_char_pointer) {
			llvm::Value* extra_size = llvm::ConstantInt::get(m_context, llvm::APInt(64, 1)); // space for a null terminator
			total_size = m_builder.CreateAdd(total_size, extra_size);
		}

		llvm::Value* allocated_ptr = m_builder.CreateCall(malloc_func, total_size);

		// cast the result to the correct pointer type
		llvm::Value* typed_ptr = m_builder.CreateBitCast(allocated_ptr, llvm::PointerType::getUnqual(element_type));

		// add null terminator if the element type is a character
		if (is_char_pointer) {
			llvm::Value* null_terminator_ptr = m_builder.CreateGEP(element_type, typed_ptr, element_count_cast);
			m_builder.CreateStore(llvm::ConstantInt::get(m_context, llvm::APInt(8, 0)), null_terminator_ptr);
		}

		value_ptr array_value = std::make_shared<value>("__alloca", array_element_type.get_pointer_type(), typed_ptr);
		array_value->set_pointer(allocated_ptr);
		return array_value;
	}

	acceptation_result codegen_visitor::visit_array_access_node(
		array_access_node& node, 
		const codegen_context& context
	) {
		const std::vector<channel::node*>& index_nodes = node.get_array_element_index_nodes();

		// evaluate the array base expression
		acceptation_result array_ptr_result = node.get_array_base_node()->accept(*this, {});
		if (!array_ptr_result.has_value()) {
			return array_ptr_result;
		}

		// traverse the array indexes
		type current_type = array_ptr_result.value()->get_type();
		llvm::Value* current_ptr = array_ptr_result.value()->get_value();

		for (size_t i = 0; i < index_nodes.size(); ++i) {
			acceptation_result index_value_result = index_nodes[i]->accept(*this, {});
			if (!index_value_result.has_value()) {
				return index_value_result;
			}

			// cast the index value to u64
			llvm::Value* index_value_cast = cast_value(index_value_result.value(), type(type::base::u64, 0), node.get_declared_position());

			// load the actual pointer value
			current_ptr = m_builder.CreateLoad(current_type.get_llvm_type(m_context), current_ptr);

			// get the next level pointer
			current_ptr = m_builder.CreateInBoundsGEP(current_type.get_element_type().get_llvm_type(m_context), current_ptr, index_value_cast);

			// update the current_type for the next iteration
			if (i != index_nodes.size() - 1) {
				current_type = current_type.get_element_type();
			}
		}

		// load the value at the final element address
		llvm::Value* loaded_value = m_builder.CreateLoad(current_type.get_element_type().get_llvm_type(m_context), current_ptr);

		value_ptr element_value = std::make_shared<value>("__array_element", current_type.get_element_type(), loaded_value);
		element_value->set_pointer(current_ptr);
		return element_value;
	}

	acceptation_result codegen_visitor::visit_array_assignment_node(
		array_assignment_node& node,
		const codegen_context& context
	) {
		const std::vector<channel::node*>& index_nodes = node.get_array_element_index_nodes();

		// evaluate the array base expression
		acceptation_result array_ptr_result = node.get_array_base_node()->accept(*this, {});
		if (!array_ptr_result.has_value()) {
			return array_ptr_result;
		}

		// traverse the array indexes
		type current_type = array_ptr_result.value()->get_type();
		llvm::Value* current_ptr = array_ptr_result.value()->get_value();

		for (size_t i = 0; i < index_nodes.size(); ++i) {
			acceptation_result index_value_result = index_nodes[i]->accept(*this, {});
			if (!index_value_result.has_value()) {
				return index_value_result;
			}

			// cast the index value to u64
			llvm::Value* index_value_cast = cast_value(index_value_result.value(), type(type::base::u64, 0), node.get_declared_position());

			// load the actual pointer value
			current_ptr = m_builder.CreateLoad(current_type.get_llvm_type(m_context), current_ptr);

			// get the next level pointer
			// current_ptr = m_builder.CreateGEP(current_type.get_element_type().get_llvm_type(m_context), current_ptr, index_value_cast);
			current_ptr = m_builder.CreateInBoundsGEP(current_type.get_element_type().get_llvm_type(m_context), current_ptr, index_value_cast);

			// update the current_type for the next iteration
			if (i != index_nodes.size() - 1) {
				current_type = current_type.get_element_type();
			}
		}

		// evaluate the right-hand side expression
		acceptation_result expression_value_result = node.get_expression_node()->accept(*this, codegen_context(current_type.get_element_type()));
		if (!expression_value_result.value()) {
			return expression_value_result;
		}

		// get the final element type for the assignment
		const type final_element_type = current_type.get_element_type();

		// cast the expression value to the array element type
		llvm::Value* expression_llvm_value_cast = cast_value(expression_value_result.value(), final_element_type, node.get_declared_position());

		value_ptr expression_value = std::make_shared<value>(expression_value_result.value()->get_name(), final_element_type, expression_llvm_value_cast);

		// store the result of the right-hand side expression in the array
		m_builder.CreateStore(expression_value_result.value()->get_value(), current_ptr);
		expression_value->set_pointer(current_ptr);
		return expression_value;
	}

	acceptation_result codegen_visitor::visit_variable_node(
		variable_node& node, 
		const codegen_context& context
	) {
		// find the variable value in our named values
		value_ptr var_value;
		if (!get_named_value(var_value, node.get_variable_identifier())) {
			return std::unexpected(error::emit<4003>(node.get_declared_position(), node.get_variable_identifier()));
		}

		// since the variable_node represents an address, we do not need to load it
		// just return the found value
		return var_value;
	}

	acceptation_result codegen_visitor::get_declaration_value(
		const declaration_node& node,
		const codegen_context& context
	) {
		// evaluate the expression to get the initial value
		if (channel::node* expression = node.get_expression_node()) {
			// evaluate the assigned value
			return expression->accept(*this, context);
		}

		// declared without an assigned value, set it to 0
		llvm::Type* value_type = node.get_declaration_type().get_llvm_type(m_context);
		return std::make_shared<value>(node.get_declaration_identifier(), node.get_declaration_type(), llvm::Constant::getNullValue(value_type));
	}
}
