#include "code_generator.h"

#include "code_generator/abstract_syntax_tree/variables/assignment_node.h"
#include "code_generator/abstract_syntax_tree/variables/variable_access_node.h"
#include "code_generator/abstract_syntax_tree/variables/variable_node.h"
#include "code_generator/abstract_syntax_tree/variables/array/array_allocation_node.h"
#include "code_generator/abstract_syntax_tree/variables/array/array_assignment_node.h"
#include "code_generator/abstract_syntax_tree/variables/array/array_access_node.h"
#include "code_generator/abstract_syntax_tree/variables/declaration/local_declaration_node.h"
#include "code_generator/abstract_syntax_tree/variables/declaration/global_declaration_node.h"

namespace sigma {
	outcome::result<value_ptr> code_generator::visit_assignment_node(
		assignment_node& node, 
		const code_generation_context& context
	) {
		SUPPRESS_C4100(context);
		
		// assignment to a local variable
		// look up the local variable in the active scope
		OUTCOME_TRY(const auto variable_result, node.get_variable_node()->accept(*this, {}));
		
		// evaluate the expression on the right-hand side of the assignment
		OUTCOME_TRY(auto expression_result, node.get_expression_node()->accept(
			*this,
			code_generation_context(variable_result->get_type())
		));
		
		llvm::Value* out_cast = cast_value(
			expression_result,
			variable_result->get_type(), 
			node.get_declared_position()
		);
		
		expression_result->set_value(out_cast);
		
		m_context->get_builder().CreateStore(
			expression_result->get_value(),
			variable_result->get_value()
		);
		
		return expression_result;
	}

	outcome::result<value_ptr> code_generator::visit_variable_access_node(
		variable_access_node& node, 
		const code_generation_context& context
	) {
		SUPPRESS_C4100(context);

		// load a local variable
		// look up the local variable in the active scope
		if (const variable_ptr variable = m_context->get_variable_registry().get_local_variable(
			node.get_identifier())
		) {
			const value_ptr variable_value = variable->get_value();

			// load the value from the memory location
			llvm::AllocaInst* alloca = llvm::dyn_cast<llvm::AllocaInst>(
				variable_value->get_value()
			);

			llvm::Value* load = m_context->get_builder().CreateLoad(
				alloca->getAllocatedType(),
				variable_value->get_value()
			);

			// return the load instruction as a value
			value_ptr variable_load = std::make_shared<value>(
				node.get_identifier(), 
				variable_value->get_type(), 
				load
			);

			variable_load->set_pointer(alloca);
			return variable_load;
		}

		// we haven't found a local variable, check if we're loading a global one
		// load a global variable
		// look up the global variable in the m_global_named_values map
		const variable_ptr global_variable = m_context->get_variable_registry().get_global_variable(node.get_identifier());
		// check if the global variable exists
		if (!global_variable) {
			return outcome::failure(
				error::emit<4003>(
					node.get_declared_position(), 
					node.get_identifier()
				)
			); // return on failure
		}

		const value_ptr global_variable_value = global_variable->get_value();

		// get the value from the global variable pointer
		const llvm::GlobalValue* global_variable_value_ptr = llvm::dyn_cast<llvm::GlobalValue>(
			global_variable_value->get_value()
		);

		// load the value from the memory location
		llvm::Value* load = m_context->get_builder().CreateLoad(
			global_variable_value_ptr->getValueType(),
			global_variable_value->get_value()
		);

		// return the load instruction as a value
		return std::make_shared<value>(
			node.get_identifier(),
			global_variable_value->get_type(),
			load
		);
	}

	outcome::result<value_ptr> code_generator::visit_local_declaration_node(
		local_declaration_node& node, 
		const code_generation_context& context
	) {
		SUPPRESS_C4100(context);

		llvm::BasicBlock* original_entry_block = m_context->get_builder().GetInsertBlock();
		llvm::Function* parent_function = original_entry_block->getParent();
		llvm::BasicBlock* function_entry_block = &*parent_function->begin();

		m_context->get_builder().SetInsertPoint(
			function_entry_block,
			function_entry_block->getFirstInsertionPt()
		);

		// store the initial value
		llvm::AllocaInst* alloca = m_context->get_builder().CreateAlloca(
			node.get_declaration_type().get_llvm_type(m_context->get_context()),
			nullptr
		);

		// add the variable to the active scope
		if(!m_context->get_variable_registry().insert_local_variable(
			node.get_declaration_identifier(),
			std::make_shared<variable>(
				std::make_shared<value>(
					node.get_declaration_identifier(),
					node.get_declaration_type(),
					alloca
				),
				node.get_declared_position()
			)
		)) {
			// insertion operation failed - the variable has already been defined before
			return outcome::failure(
				error::emit<4004>(
					node.get_declared_position(),
					node.get_declaration_identifier()
				)
			);
		}

		// assign the actual value
		m_context->get_builder().SetInsertPoint(original_entry_block);

		// evaluate the assigned value, if there is one
		OUTCOME_TRY(auto declaration_value_result, get_declaration_value(
			node,
			code_generation_context(node.get_declaration_type())
		));

		llvm::Value* cast_assigned_value = cast_value(
			declaration_value_result,
			node.get_declaration_type(),
			node.get_declared_position()
		);

		m_context->get_builder().CreateStore(cast_assigned_value, alloca);
		declaration_value_result->set_pointer(alloca);
		m_context->get_builder().SetInsertPoint(original_entry_block);
		return declaration_value_result;
	}

	outcome::result<value_ptr> code_generator::visit_global_declaration_node(
		global_declaration_node& node,
		const code_generation_context& context
	) {
		SUPPRESS_C4100(context);

		// start creating the init function for our global ctor
		const std::string init_func_name = "__global_init_" + node.get_declaration_identifier();
		llvm::FunctionType* init_func_type = llvm::FunctionType::get(
			llvm::Type::getVoidTy(m_context->get_context()),
			false
		);

		llvm::Function* init_func = llvm::Function::Create(
			init_func_type,
			llvm::Function::ExternalLinkage,
			init_func_name,
			m_context->get_module().get()
		);

		llvm::BasicBlock* init_func_entry = llvm::BasicBlock::Create(
			m_context->get_context(),
			"",
			init_func
		);

		m_context->get_builder().SetInsertPoint(init_func_entry); // write to the init function

		// evaluate the assigned value, if there is one
		OUTCOME_TRY(auto declaration_value_result, get_declaration_value(
			node,
			code_generation_context(node.get_declaration_type())
		));

		// cast the right-hand side operator to the assigned type
		llvm::Value* cast_assigned_value = cast_value(
			declaration_value_result,
			node.get_declaration_type(),
			node.get_declared_position()
		);

		// create a global variable
		value_ptr global_declaration = std::make_shared<value>(
			node.get_declaration_identifier(),
			node.get_declaration_type(),
			new llvm::GlobalVariable(*m_context->get_module(),
				node.get_declaration_type().get_llvm_type(m_context->get_context()),
				false,
				llvm::GlobalValue::ExternalLinkage,
				llvm::Constant::getNullValue(
					node.get_declaration_type().get_llvm_type(m_context->get_context())
				), // default initializer
				node.get_declaration_identifier()
			)
		);

		// add the variable to the m_global_named_values map
		if (!m_context->get_variable_registry().insert_global_variable(
			node.get_declaration_identifier(),
			std::make_shared<variable>(
				global_declaration,
				node.get_declared_position()
			)
		)) {
			// variable insertion failed - variable has been declared before
			return outcome::failure(
				error::emit<4006>(
					node.get_declared_position(),
					node.get_declaration_identifier()
				)
			); // return on failure
		}

		m_context->get_builder().CreateStore(
			cast_assigned_value,
			global_declaration->get_value()
		);

		global_declaration->set_pointer(
			global_declaration->get_value()
		);

		m_context->get_builder().CreateRetVoid();

		// create a new constructor with the given priority
		llvm::ConstantInt* priority = llvm::ConstantInt::get(
			llvm::Type::getInt32Ty(m_context->get_context()),
			m_context->get_variable_registry().increment_global_initialization_priority()
		);

		llvm::Constant* initializer_cast = llvm::ConstantExpr::getBitCast(
			init_func,
			llvm::Type::getInt8PtrTy(m_context->get_context())
		);

		llvm::Constant* new_ctor = llvm::ConstantStruct::get(llvm::StructType::get(
			m_context->get_context(), {
				llvm::Type::getInt32Ty(m_context->get_context()),
				llvm::Type::getInt8PtrTy(m_context->get_context()),
				llvm::Type::getInt8PtrTy(m_context->get_context())
			}
		), {
			priority,
			initializer_cast,
			llvm::Constant::getNullValue(llvm::Type::getInt8PtrTy(m_context->get_context()))
			});

		// Check if the global ctors array exists
		llvm::GlobalVariable* global_ctors_var = m_context->get_module()->getGlobalVariable("llvm.global_ctors");

		std::vector<llvm::Constant*> all_ctors;
		if (global_ctors_var) {
			// If the global variable already exists, get all ctors from it
			if (auto* initializer = dyn_cast<llvm::ConstantArray>(global_ctors_var->getInitializer())) {
				for (unsigned i = 0, e = initializer->getNumOperands(); i != e; ++i) {
					all_ctors.push_back(cast<llvm::Constant>(initializer->getOperand(i)));
				}
			}
		}

		// Add new_ctor to all_ctors
		all_ctors.push_back(new_ctor);

		// Create or update the global ctors array
		llvm::ArrayType* ctor_array_type = llvm::ArrayType::get(llvm::StructType::get(
			m_context->get_context(), {
				llvm::Type::getInt32Ty(m_context->get_context()),
				llvm::Type::getInt8PtrTy(m_context->get_context()),
				llvm::Type::getInt8PtrTy(m_context->get_context())
			}
		), all_ctors.size());

		llvm::Constant* updated_ctors = llvm::ConstantArray::get(ctor_array_type, all_ctors);

		if (!global_ctors_var) {
			// If the global variable does not exist, create a new one
			global_ctors_var = new llvm::GlobalVariable(*m_context->get_module(), ctor_array_type, false, llvm::GlobalValue::AppendingLinkage, updated_ctors, "llvm.global_ctors");
		}
		else {
			// If the global variable already exists, update it
			global_ctors_var->setInitializer(updated_ctors);
		}

		return global_declaration;
	}

	outcome::result<value_ptr> code_generator::visit_allocation_node(
		array_allocation_node& node,
		const code_generation_context& context
	) {
		SUPPRESS_C4100(context);

		// get the count of allocated elements
		OUTCOME_TRY(auto element_count_result, node.get_array_element_count_node()->accept(
			*this,
			{}
		));

		// cast the element count node to u64
		llvm::Value* element_count_cast = cast_value(
			element_count_result, 
			type(type::base::u64, 0),
			node.get_array_element_count_node()->get_declared_position()
		);

		const llvm::FunctionCallee malloc_func = m_context->get_function_registry().get_function(
			"malloc",
			m_context
		)->get_function();

		// calculate the total size
		const type array_element_type = node.get_array_element_type();
		llvm::Type* element_type = array_element_type.get_llvm_type(
			m_context->get_context()
		);

		llvm::Value* element_size = llvm::ConstantInt::get(
			m_context->get_context(), 
			llvm::APInt(64, (node.get_array_element_type().get_bit_width() + 7) / 8)
		);

		llvm::Value* total_size = m_context->get_builder().CreateMul(
			element_count_cast,
			element_size
		);

		// check if we have a char*
		const bool is_char_pointer = array_element_type.get_base() == type::base::character && array_element_type.get_pointer_level() == 0;

		// add space for a null terminator if the element type is a character
		if (is_char_pointer) {
			llvm::Value* extra_size = llvm::ConstantInt::get(
				m_context->get_context(), 
				llvm::APInt(64, 1)
			); // space for a null terminator

			total_size = m_context->get_builder().CreateAdd(
				total_size, 
				extra_size
			);
		}

		// allocate the array
		llvm::Value* allocated_ptr = m_context->get_builder().CreateCall(
			malloc_func, 
			total_size
		);

		// cast the result to the correct pointer type
		llvm::Value* typed_ptr = m_context->get_builder().CreateBitCast(
			allocated_ptr,
			llvm::PointerType::getUnqual(element_type)
		);

		// add null terminator if the element type is a character
		if (is_char_pointer) {
			llvm::Value* null_terminator_ptr = m_context->get_builder().CreateGEP(
				element_type,
				typed_ptr, 
				element_count_cast
			);

			m_context->get_builder().CreateStore(llvm::ConstantInt::get(m_context->get_context(), llvm::APInt(8, 0)), null_terminator_ptr);
		}

		value_ptr array_value = std::make_shared<value>(
			"__alloca",
			array_element_type.get_pointer_type(),
			typed_ptr
		);

		array_value->set_pointer(
			allocated_ptr
		);

		return array_value;
	}

	outcome::result<value_ptr> code_generator::visit_array_access_node(
		array_access_node& node, 
		const code_generation_context& context
	) {
		SUPPRESS_C4100(context);

		const std::vector<node_ptr>& index_nodes = node.get_array_element_index_nodes();

		// evaluate the array base expression
		OUTCOME_TRY(auto array_ptr_result, node.get_array_base_node()->accept(*this, {}));

		// traverse the array indexes
		type current_type = array_ptr_result->get_type();
		llvm::Value* current_ptr = array_ptr_result->get_value();

		for (u64 i = 0; i < index_nodes.size(); ++i) {
			OUTCOME_TRY(auto index_value_result, index_nodes[i]->accept(*this, {}));

			// cast the index value to u64
			llvm::Value* index_value_cast = cast_value(
				index_value_result,
				type(type::base::u64, 0),
				node.get_declared_position()
			);

			// load the actual pointer value
			current_ptr = m_context->get_builder().CreateLoad(
				current_type.get_llvm_type(m_context->get_context()), 
				current_ptr
			);

			// get the next level pointer
			current_ptr = m_context->get_builder().CreateInBoundsGEP(
				current_type.get_element_type().get_llvm_type(m_context->get_context()),
				current_ptr,
				index_value_cast
			);

			// update the current_type for the next iteration
			if (i != index_nodes.size() - 1) {
				current_type = current_type.get_element_type();
			}
		}

		// load the value at the final element address
		llvm::Value* loaded_value = m_context->get_builder().CreateLoad(
			current_type.get_element_type().get_llvm_type(m_context->get_context()), 
			current_ptr
		);

		value_ptr element_value = std::make_shared<value>(
			"__array_element", 
			current_type.get_element_type(), 
			loaded_value
		);

		element_value->set_pointer(current_ptr);
		return element_value;
	}

	outcome::result<value_ptr> code_generator::visit_array_assignment_node(
		array_assignment_node& node,
		const code_generation_context& context
	) {
		SUPPRESS_C4100(context);
		
		const std::vector<node_ptr>& index_nodes = node.get_array_element_index_nodes();
		
		// evaluate the array base expression
		OUTCOME_TRY(const auto& array_ptr_result, node.get_array_base_node()->accept(*this, {}));
		
		// traverse the array indexes
		type current_type = array_ptr_result->get_type();
		llvm::Value* current_ptr = array_ptr_result->get_value();
		
		for (u64 i = 0; i < index_nodes.size(); ++i) {
			OUTCOME_TRY(const auto& index_value_result, index_nodes[i]->accept(*this, {}));
		
			// cast the index value to u64
			llvm::Value* index_value_cast = cast_value(
				index_value_result,
				type(type::base::u64, 0), 
				node.get_declared_position()
			);
		
			// load the actual pointer value
			current_ptr = m_context->get_builder().CreateLoad(
				current_type.get_llvm_type(m_context->get_context()), 
				current_ptr
			);
		
			// get the next level pointer
			// current_ptr = m_llvm_handler->get_builder().CreateGEP(current_type.get_element_type().get_llvm_type(m_llvm_handler->get_context()), current_ptr, index_value_cast);
			current_ptr = m_context->get_builder().CreateInBoundsGEP(
				current_type.get_element_type().get_llvm_type(m_context->get_context()),
				current_ptr, 
				index_value_cast
			);
		
			// update the current_type for the next iteration
			if (i != index_nodes.size() - 1) {
				current_type = current_type.get_element_type();
			}
		}
		
		// evaluate the right-hand side expression
		OUTCOME_TRY(auto expression_value_result, node.get_expression_node()->accept(
			*this,
			code_generation_context(current_type.get_element_type())
		));
		
		// get the final element type for the assignment
		const type final_element_type = current_type.get_element_type();
		
		// cast the expression value to the array element type
		llvm::Value* expression_llvm_value_cast = cast_value(
			expression_value_result, 
			final_element_type, 
			node.get_declared_position()
		);
		
		value_ptr expression_value = std::make_shared<value>(
			expression_value_result->get_name(),
			final_element_type,
			expression_llvm_value_cast
		);
		
		// store the result of the right-hand side expression in the array
		m_context->get_builder().CreateStore(
			expression_value_result->get_value(), 
			current_ptr
		);
		
		expression_value->set_pointer(current_ptr);
		return expression_value;

		return nullptr;
	}

	outcome::result<value_ptr> code_generator::visit_variable_node(
		variable_node& node, 
		const code_generation_context& context
	) {
		SUPPRESS_C4100(context);

		// find the variable value in our named values
		if (const variable_ptr variable = m_context->get_variable_registry().get_variable(node.get_identifier())) {
			// since the variable_node represents an address, we do not need to load it
			// just return the found value
			return variable->get_value();
		}

		return outcome::failure(
			error::emit<4003>(
				node.get_declared_position(),
				node.get_identifier()
			)
		); // return on failure
	}

	outcome::result<value_ptr> code_generator::get_declaration_value(
		const declaration_node& node,
		const code_generation_context& context
	) {
		// evaluate the expression to get the initial value
		if (const node_ptr expression = node.get_expression_node()) {
			// evaluate the assigned value
			return expression->accept(*this, context);
		}

		// declared without an assigned value, set it to 0
		llvm::Type* value_type = node.get_declaration_type().get_llvm_type(
			m_context->get_context()
		);

		return std::make_shared<value>(
			node.get_declaration_identifier(),
			node.get_declaration_type(),
			llvm::Constant::getNullValue(value_type)
		);
	}
}
