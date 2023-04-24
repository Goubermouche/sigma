#include "codegen_visitor.h"

#include "../abstract_syntax_tree/functions/function_call_node.h"
#include "../abstract_syntax_tree/functions/function_node.h"

namespace channel {
	bool codegen_visitor::visit_function_node(function_node& node, value*& out_value) {
		// get the function return type
		llvm::Type* return_type = node.get_function_return_type().get_llvm_type(m_context);

		// convert the argument types to LLVM types and store them in a vector
		std::vector<llvm::Type*> param_types;
		for (const auto& [arg_name, arg_type] : node.get_function_arguments()) {
			param_types.push_back(arg_type.get_llvm_type(m_context));
		}

		// create the LLVM function
		llvm::FunctionType* func_type = llvm::FunctionType::get(return_type, param_types, false);
		llvm::Function* func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, node.get_function_identifier(), m_module.get());

		// add it to our function map
		const auto insertion_result = m_functions.insert({
			node.get_function_identifier(),
			new function(node.get_function_return_type(), func, node.get_function_arguments(), false)
		});

		// check for multiple definitions by checking if the function has already been added to our map
		if (!insertion_result.second) {
			compilation_logger::emit_function_already_defined_error(node.get_declaration_line_number(), node.get_function_identifier());
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
		for (const auto& [arg_name, arg_type] : node.get_function_arguments()) {
			// get the corresponding LLVM function argument
			llvm::Argument* llvm_arg = func->arg_begin() + index;
			llvm_arg->setName(arg_name);

			// create an alloca instruction for the argument and store the incoming value into it
			llvm::AllocaInst* alloca = m_builder.CreateAlloca(arg_type.get_llvm_type(m_context), nullptr, arg_name);
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
		for (const auto& statement : node.get_function_statements()) {
			value* temp_statement_value;
			if (!statement->accept(*this, temp_statement_value)) {
				return false;
			}
		}
	
		// restore the previous scope
		m_scope = prev_scope;

		// add a return statement if the function does not have one
		if (entry_block->getTerminator() == nullptr) {
			compilation_logger::emit_function_return_auto_generate_warning(node.get_declaration_line_number(), node.get_function_identifier());

			if (return_type->isVoidTy()) {
				m_builder.CreateRetVoid();
			}
			else {
				m_builder.CreateRet(llvm::Constant::getNullValue(return_type));
			}
		}

		// return the function as the value
		out_value = new value(node.get_function_identifier(), type(type::base::function, 0), func);
		return true;
	}

	bool codegen_visitor::visit_function_call_node(function_call_node& node, value*& out_value) {
		// get a reference to the function
		const function* func = m_functions[node.get_function_identifier()];

		// check if it exists
		if (!func) {
			compilation_logger::emit_function_not_found_error(node.get_declaration_line_number(), node.get_function_identifier());
			return false;
		}

		const std::vector<std::pair<std::string, type>>& required_arguments = func->get_arguments();
		const std::vector<channel::node*>& given_arguments = node.get_function_arguments();

		if(!func->is_variadic() && required_arguments.size() != given_arguments.size()) {
			compilation_logger::emit_function_argument_count_mismatch_error(node.get_declaration_line_number(), node.get_function_identifier());
			return false;
		}

		std::vector<llvm::Value*> argument_values(required_arguments.size());

		for (u64 i = 0; i < required_arguments.size(); i++) {
			// get the argument value
			value* argument_value;
			if(!given_arguments[i]->accept(*this, argument_value)) {
				return false;
			}

			// cast the given argument to match the required argument's type, if necessary 
			if(!cast_value(argument_values[i], argument_value, required_arguments[i].second, node.get_declaration_line_number())) {
				return false;
			}
		}

		for (u64 i = required_arguments.size(); i < given_arguments.size(); i++) {
			// get the argument value
			value* argument_value;
			if (!given_arguments[i]->accept(*this, argument_value)) {
				return false;
			}

			// default variadic promotions:
			const type argument_type = argument_value->get_type();
			if(argument_type == type(type::base::f32, 0)) {
				// f32 -> f64
				llvm::Value* argument_value_cast;
				if(!cast_value(argument_value_cast, argument_value, type(type::base::f64, 0), given_arguments[i]->get_declaration_line_number())) {
					return false;
				}

				argument_value = new value(argument_value->get_name(), type(type::base::f64, 0), argument_value_cast);
			}
			else if(argument_type.get_bit_width() < type(type::base::i32, 0).get_bit_width()) {
				// i1, i8, i16 -> i32
				llvm::Value* argument_value_cast;
				if (!cast_value(argument_value_cast, argument_value, type(type::base::i32, 0), given_arguments[i]->get_declaration_line_number())) {
					return false;
				}

				argument_value = new value(argument_value->get_name(), type(type::base::i32, 0), argument_value_cast);
			}

			argument_values.push_back(argument_value->get_value());
		}

		// return the function call as the value
		llvm::CallInst* call_inst = m_builder.CreateCall(func->get_function(), argument_values);
		const type return_type = func->get_return_type();

		// only return the call if we have to store the value (if the function returns a non-void and non-pointer value)
		if (return_type.is_pointer() || return_type.get_base() != type::base::empty) {
			out_value = new value(node.get_function_identifier(), return_type, call_inst);
		}
		else {
			out_value = nullptr;
		}

		return true;
	}
}