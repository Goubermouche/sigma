#include "codegen_visitor.h"

#include "../abstract_syntax_tree/functions/function_call_node.h"
#include "../abstract_syntax_tree/functions/function_node.h"

namespace channel {
	bool codegen_visitor::visit_function_node(function_node& node, value*& out_value) {
		// get the function return type
		llvm::Type* return_type = type_to_llvm_type(node.get_return_type(), m_context);
		llvm::FunctionType* func_type = llvm::FunctionType::get(return_type, false);

		// create the LLVM function
		llvm::Function* func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, node.get_name(), m_module.get());

		// add it to our function map
		const auto insertion_result = m_functions.insert({
			node.get_name(),
			new function(node.get_return_type(), func)
		});

		// check for multiple definitions by checking if the function has already been added to our map
		if (!insertion_result.second) {
			compilation_logger::emit_function_already_defined_error(node.get_declaration_line_number(), node.get_name());
			return false;
		}

		// create and use a new entry block
		llvm::BasicBlock* entry_block = llvm::BasicBlock::Create(m_context, "entry", func);
		m_builder.SetInsertPoint(entry_block);

		// create a new nested scope for the function body
		scope* prev_scope = m_scope;
		m_scope = new scope(prev_scope);

		// accept all statements inside the function
		for (const auto& statement : node.get_statements()) {
			value* temp_statement_value;
			if (!statement->accept(*this, temp_statement_value)) {
				return false;
			}
		}

		// restore the previous scope
		m_scope = prev_scope;

		// add a return statement if the function does not have one
		if (entry_block->getTerminator() == nullptr) {
			compilation_logger::emit_function_return_auto_generate_warning(node.get_declaration_line_number(), node.get_name());

			if (return_type->isVoidTy()) {
				m_builder.CreateRetVoid();
			}
			else {
				m_builder.CreateRet(llvm::Constant::getNullValue(return_type));
			}
		}

		// return the function as the value
		out_value = new value(node.get_name(), type::function, func);
		return true;
	}

	bool codegen_visitor::visit_function_call_node(function_call_node& node, value*& out_value) {
		// get a reference to the function
		llvm::Function* func = m_functions[node.get_name()]->get_function();
		// check if it exists
		if (!func) {
			compilation_logger::emit_function_not_found_error(node.get_declaration_line_number(), node.get_name());
			return false;
		}

		// todo: generate code for each argument expression
		std::vector<llvm::Value*> argument_values;
		//for (channel::node* argument : node.get_arguments()) {
		//	if(!argument->accept(*this, )) {
		//		
		//	}
		//	
		//	argument_values.push_back(m_builder.GetInsertBlock()->getParent()->back().getTerminator()->getOperand(0));
		//}

		// return the function call as the value
		out_value = new value(node.get_name(), type::function_call, m_builder.CreateCall(func, argument_values, "call"));
		return true;
	}
}