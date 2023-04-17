#include "codegen_visitor.h"

#include "../abstract_syntax_tree/functions/function_call_node.h"
#include "../abstract_syntax_tree/functions/function_node.h"

namespace channel {
	bool codegen_visitor::visit_function_node(function_node& node, value*& out_value) {
		// get the function return type
		llvm::Type* return_type = type_to_llvm_type(node.get_return_type(), m_context);

		// convert the argument types to LLVM types and store them in a vector
		std::vector<llvm::Type*> param_types;
		for (const auto& [arg_name, arg_type] : node.get_arguments()) {
			param_types.push_back(type_to_llvm_type(arg_type, m_context));
		}

		// create the LLVM function
		llvm::FunctionType* func_type = llvm::FunctionType::get(return_type, param_types, false);
		llvm::Function* func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, node.get_name(), m_module.get());

		// add it to our function map
		const auto insertion_result = m_functions.insert({
			node.get_name(),
			new function(node.get_return_type(), func, node.get_arguments())
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

		// create the given arguments 
		u64 index = 0;
		for (const auto& [arg_name, arg_type] : node.get_arguments()) {
			// get the corresponding LLVM function argument
			llvm::Argument* llvm_arg = func->arg_begin() + index;
			llvm_arg->setName(arg_name);

			// create an alloca instruction for the argument and store the incoming value into it
			llvm::AllocaInst* alloca = m_builder.CreateAlloca(type_to_llvm_type(arg_type, m_context), nullptr, arg_name);
			m_builder.CreateStore(llvm_arg, alloca);


			// add the alloca to the current scope
			m_scope->add_named_value(arg_name, new value(
				arg_name,
				arg_type,
				alloca
			));

			index++;
		}

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
		const function* func = m_functions[node.get_name()];

		// temp: create a system for importing C functions.
		if(node.get_name() == "print") {
			value* argument_value;
			if (!node.get_arguments()[0]->accept(*this, argument_value)) {
				return false;
			}

			llvm::Value* printf_format = m_builder.CreateGlobalStringPtr("%d\n", "printf_format");
			std::vector<llvm::Value*> printf_args = { printf_format, argument_value->get_value() };
			out_value = new value(node.get_name(), type::function_call, m_builder.CreateCall(func->get_function(), printf_args, "call"));
			return true;
		}

		// check if it exists
		if (!func) {
			compilation_logger::emit_function_not_found_error(node.get_declaration_line_number(), node.get_name());
			return false;
		}

		// todo: generate code for each argument expression
		const std::vector<std::pair<std::string, type>>& arguments = func->get_arguments();
		const std::vector<channel::node*>& given_arguments = node.get_arguments();

		if(arguments.size() != given_arguments.size()) {
			compilation_logger::emit_function_argument_count_mismatch_error(node.get_declaration_line_number(), node.get_name());
			return false;
		}

		std::vector<llvm::Value*> argument_values(arguments.size());

		for (u64 i = 0; i < arguments.size(); i++) {
			// get the argument value
			value* argument_value;
			if(!given_arguments[i]->accept(*this, argument_value)) {
				return false;
			}

			// cast the given argument to match the required argument's type, if necessary 
			if(!cast_value(argument_values[i], argument_value, arguments[i].second, node.get_declaration_line_number())) {
				return false;
			}
		}

		// return the function call as the value
		out_value = new value(node.get_name(), type::function_call, m_builder.CreateCall(func->get_function(), argument_values, "call"));
		return true;
	}
}