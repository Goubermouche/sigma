#include "codegen_visitor.h"

#include "../abstract_syntax_tree/variables/function_call_node.h"
#include "../abstract_syntax_tree/variables/function_node.h"

namespace channel {
	bool codegen_visitor::visit_function_node(function_node& node, value*& out_value) {
		llvm::Type* return_type = type_to_llvm_type(node.get_return_type(), m_context);
		llvm::FunctionType* function_type = llvm::FunctionType::get(return_type, false);
		llvm::Function* func = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, node.get_name(), m_module.get());

		const auto insertion_result = m_functions.insert({
			node.get_name(),
			new function(node.get_return_type(), func)
			});

		if (!insertion_result.second) {
			compilation_logger::emit_function_already_defined_error(node.get_declaration_line_index(), node.get_name());
			return false;
		}

		llvm::BasicBlock* entry_block = llvm::BasicBlock::Create(m_context, "entry", func);
		m_builder.SetInsertPoint(entry_block);

		// create a new nested scope for the function body
		scope* prev_scope = m_scope;
		m_scope = new scope(prev_scope);

		// accept all statements inside the function
		for (const auto& statement : node.get_statements()) {
			value* statement_value;
			if (!statement->accept(*this, statement_value)) {
				return false;
			}
		}

		// restore the previous scope
		m_scope = prev_scope;

		// add a return statement if the function does not have one
		if (entry_block->getTerminator() == nullptr) {
			compilation_logger::emit_function_return_auto_generate_warning(node.get_declaration_line_index(), node.get_name());

			if (return_type->isVoidTy()) {
				m_builder.CreateRetVoid();
			}
			else {
				m_builder.CreateRet(llvm::Constant::getNullValue(return_type));
			}
		}

		out_value = new value(node.get_name(), type::function, func);
		return true;
	}

	bool codegen_visitor::visit_function_call_node(function_call_node& node, value*& out_value) {
		llvm::Function* function = m_functions[node.get_name()]->get_function();
		if (!function) {
			compilation_logger::emit_function_not_found_error(node.get_declaration_line_index(), node.get_name());
			return false;
		}

		// generate code for each argument expression
		std::vector<llvm::Value*> argument_values;
		//for (channel::node* argument : node.get_arguments()) {
		//	if(!argument->accept(*this, )) {
		//		
		//	}
		//	
		//	argument_values.push_back(m_builder.GetInsertBlock()->getParent()->back().getTerminator()->getOperand(0));
		//}

		out_value = new value(node.get_name(), type::function_call, m_builder.CreateCall(function, argument_values, "call"));
		return true;
	}
}