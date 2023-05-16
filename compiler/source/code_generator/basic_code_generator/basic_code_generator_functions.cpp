#include "basic_code_generator.h"

#include "code_generator/abstract_syntax_tree/functions/function_call_node.h"
#include "code_generator/abstract_syntax_tree/functions/function_node.h"

namespace channel {
	acceptation_result basic_code_generator::visit_function_node(
		function_node& node, 
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		// get the function return type
		llvm::Type* return_type = node.get_function_return_type().get_llvm_type(
			m_llvm_context->get_context()
		);

		// convert the argument types to LLVM types and store them in a vector
		std::vector<llvm::Type*> param_types;
		for (const auto& [arg_name, arg_type] : node.get_function_arguments()) {
			param_types.push_back(
				arg_type.get_llvm_type(
					m_llvm_context->get_context()
				)
			);
		}

		// create the LLVM function
		llvm::FunctionType* func_type = llvm::FunctionType::get(
			return_type, 
			param_types, 
			false
		);

		llvm::Function* func = llvm::Function::Create(
			func_type, 
			llvm::Function::ExternalLinkage,
			node.get_function_identifier(), 
			m_llvm_context->get_module().get()
		);

		// check for multiple definitions by checking if the function has already been added to our map
		if(m_function_registry.contains_function(
			node.get_function_identifier()
		)) {
			return std::unexpected(
				error::emit<4000>(
					node.get_declared_position(), 
					node.get_function_identifier()
				)
			); // return on failure
		}

		// add it to our function map
		m_function_registry.insert_function(
			node.get_function_identifier(),
			std::make_shared<function>(
				node.get_function_return_type(),
				func,
				node.get_function_arguments(),
				false
			)
		);

		// create and use a new entry block
		llvm::BasicBlock* entry_block = llvm::BasicBlock::Create(
			m_llvm_context->get_context(),
			"",
			func
		);

		m_llvm_context->get_builder().SetInsertPoint(entry_block);

		// create a new nested scope for the function body
		scope_ptr prev_scope = m_scope;
		m_scope = std::make_shared<scope>(prev_scope, nullptr);

		// create the given arguments 
		u64 index = 0;
		for (const auto& [arg_name, arg_type] : node.get_function_arguments()) {
			// get the corresponding LLVM function argument
			llvm::Argument* llvm_arg = func->arg_begin() + index;
			llvm_arg->setName(arg_name);

			// create an alloca instruction for the argument and store the incoming value into it
			llvm::AllocaInst* alloca = m_llvm_context->get_builder().CreateAlloca(
				arg_type.get_llvm_type(m_llvm_context->get_context()), 
				nullptr, 
				arg_name
			);

			m_llvm_context->get_builder().CreateStore(llvm_arg, alloca);

			// add the alloca to the current scope
			m_scope->add_named_value(arg_name, std::make_shared<value>(
				arg_name,
				arg_type,
				alloca
			));

			index++;
		}

		// accept all statements inside the function
		for (const auto& statement : node.get_function_statements()) {
			acceptation_result statement_result = statement->accept(
				*this,
				{}
			);

			if (!statement_result.has_value()) {
				return statement_result; // return on failure
			}
		}
	
		// restore the previous scope
		m_scope = prev_scope;

		// add a return statement if the function does not have one
		if (m_llvm_context->get_builder().GetInsertBlock()->getTerminator() == nullptr) {
			// emit the relevant warning
			// check if the return type is non-void
			if(node.get_function_return_type() != type(type::base::empty, 0)) {
				warning::emit<3000>(
					node.get_declared_position(), 
					node.get_function_identifier()
				).print();
			}

			if (return_type->isVoidTy()) {
				m_llvm_context->get_builder().CreateRetVoid();
			}
			else {
				m_llvm_context->get_builder().CreateRet(
					llvm::Constant::getNullValue(
						return_type
					)
				);
			}
		}

		// return the function as the value
		return std::make_shared<value>(
			node.get_function_identifier(),
			type(type::base::function, 0),
			func
		);
	}

	acceptation_result basic_code_generator::visit_function_call_node(
		function_call_node& node, 
		const codegen_context& context
	) {
		(void)context; // suppress C4100
		// get a reference to the function
		const function_ptr func = m_function_registry.get_function(
			node.get_function_identifier()
		);

		// check if it exists
		if (!func) {
			return std::unexpected(
				error::emit<4001>(
					node.get_declared_position(),
					node.get_function_identifier()
				)
			); // return on failure
		}

		const std::vector<std::pair<std::string, type>>& required_arguments = func->get_arguments();
		const std::vector<channel::node*>& given_arguments = node.get_function_arguments();

		if(!func->is_variadic() && required_arguments.size() != given_arguments.size()) {
			return std::unexpected(
				error::emit<4002>(
					node.get_declared_position(),
					node.get_function_identifier()
				)
			); // return on failure
		}

		std::vector<llvm::Value*> argument_values(required_arguments.size());

		for (u64 i = 0; i < required_arguments.size(); i++) {
			// get the argument value
			acceptation_result argument_result = given_arguments[i]->accept(
				*this,
				codegen_context(required_arguments[i].second)
			);

			if(!argument_result.has_value()) {
				return argument_result; // return on failure
			}

			// cast the given argument to match the required argument's type, if necessary
			argument_values[i] = cast_value(
				argument_result.value(), 
				required_arguments[i].second, 
				node.get_declared_position()
			);
		}

		// parse variadic arguments
		for (u64 i = required_arguments.size(); i < given_arguments.size(); i++) {
			// get the argument value
			acceptation_result argument_result = given_arguments[i]->accept(
				*this,
				{}
			);

			if (!argument_result.has_value()) {
				return argument_result; // return on failure
			}

			value_ptr argument_value = argument_result.value();

			const type argument_type = argument_result.value()->get_type();

			// default variadic promotions:
			if(argument_type == type(type::base::f32, 0)) {
				// f32 -> f64
				llvm::Value* argument_value_cast = cast_value(
					argument_value,
					type(type::base::f64, 0),
					given_arguments[i]->get_declared_position()
				);

				argument_value = std::make_shared<value>(
					argument_value->get_name(),
					type(type::base::f64, 0), 
					argument_value_cast
				);
			}
			else if(argument_type.get_bit_width() < type(type::base::i32, 0).get_bit_width()) {
				// i1, i8, i16 -> i32
				llvm::Value* argument_value_cast = cast_value(
					argument_value,
					type(type::base::i32, 0), 
					given_arguments[i]->get_declared_position()
				);

				argument_value = std::make_shared<value>(
					argument_value->get_name(),
					type(type::base::i32, 0),
					argument_value_cast
				);
			}

			argument_values.push_back(argument_value->get_value());
		}

		// return the function call as the value
		llvm::CallInst* call_inst = m_llvm_context->get_builder().CreateCall(
			func->get_function(),
			argument_values
		);

		const type return_type = func->get_return_type();

		// only return the call if we have to store the value (if the function returns a non-void and non-pointer value)
		if (return_type.is_pointer() || 
			return_type.get_base() != type::base::empty) {
			return std::make_shared<value>(
				node.get_function_identifier(),
				return_type, 
				call_inst
			); 
		}

		return nullptr;
	}
}