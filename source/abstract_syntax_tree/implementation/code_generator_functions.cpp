#include "code_generator.h"

#include "abstract_syntax_tree/functions/function_call_node.h"
#include "abstract_syntax_tree/functions/function_node.h"

namespace sigma {
	outcome::result<value_ptr> code_generator::visit_function_node(
		function_node& node, 
		const code_generation_context& context
	) {
		SUPPRESS_C4100(context);

		// get the function return type
		llvm::Type* return_type = node.get_function_return_type().get_llvm_type(
			m_context->get_context()
		);

		// convert the argument types to LLVM types and store them in a vector
		std::vector<llvm::Type*> param_types;
		for (const auto& [arg_name, arg_type] : node.get_function_arguments()) {
			param_types.push_back(
				arg_type.get_llvm_type(
					m_context->get_context()
				)
			);
		}

		// create the LLVM function
		llvm::FunctionType* func_type = llvm::FunctionType::get(
			return_type, 
			param_types, 
			node.is_var_arg()
		);

		llvm::Function* func = llvm::Function::Create(
			func_type, 
			llvm::Function::ExternalLinkage,
			node.get_function_identifier(), 
			m_context->get_module().get()
		);

		// add it to our function map
		if(!m_context->get_function_registry().insert_function(
			node.get_function_identifier(),
			std::make_shared<function>(
				node.get_function_return_type(),
				func,
				node.get_function_arguments(),
				false,
				node.get_declared_range()
			)
		)) {
			// failed to insert the function into the registry - function has already been defined before
			return outcome::failure(
				error::emit<error_code::function_already_defined>(
					file_range{}, // node.get_declared_position(),
					node.get_function_identifier()
				)
			); // return on failure
		}

		// create and use a new entry block
		llvm::BasicBlock* entry_block = llvm::BasicBlock::Create(
			m_context->get_context(),
			"",
			func
		);

		m_context->get_builder().SetInsertPoint(entry_block);

		// create a new nested scope for the function body
		m_context->get_variable_registry().push_scope();

		// create the given arguments 
		u64 index = 0;
		for (const auto& [arg_name, arg_type] : node.get_function_arguments()) {
			// get the corresponding LLVM function argument
			llvm::Argument* llvm_arg = func->arg_begin() + index;
			llvm_arg->setName(arg_name);

			// create an alloca instruction for the argument and store the incoming value into it
			llvm::AllocaInst* alloca = m_context->get_builder().CreateAlloca(
				arg_type.get_llvm_type(m_context->get_context()), 
				nullptr, 
				arg_name
			);

			m_context->get_builder().CreateStore(llvm_arg, alloca);

			// add the alloca to the current scope
			m_context->get_variable_registry().insert_local_variable(
				arg_name,
				std::make_shared<variable>(
					std::make_shared<value>(
						arg_name,
						arg_type,
						alloca
					),
					node.get_declared_range()
				)
			);

			index++;
		}

		// accept all statements inside the function
		for (const auto& statement : node.get_function_statements()) {
			OUTCOME_TRY(statement->accept(*this, {}));
		}
	
		// restore the previous scope
		m_context->get_variable_registry().pop_scope();

		// add a return statement if the function does not have one
		if (m_context->get_builder().GetInsertBlock()->getTerminator() == nullptr) {
			// emit the relevant warning
			// check if the return type is non-void
			if(node.get_function_return_type() != type(type::base::empty, 0)) {
				warning::emit<warning_code::implicit_function_return_generated>(
					node.get_declared_range(),
					node.get_function_identifier()
				)->print();
			}

			if (return_type->isVoidTy()) {
				m_context->get_builder().CreateRetVoid();
			}
			else {
				m_context->get_builder().CreateRet(
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

	outcome::result<value_ptr> code_generator::visit_function_call_node(
		function_call_node& node, 
		const code_generation_context& context
	) {
		SUPPRESS_C4100(context);

		const function_ptr func = m_context->get_function_registry().get_function(
			node.get_function_identifier(),
			m_context
		);

		// check if it exists
		if(!func) {
			return outcome::failure(
				error::emit<error_code::function_cannot_be_found>(
					file_range{}, //node.get_declared_position(),
					node.get_function_identifier()
				)
			); // return on failure
		}

		const std::vector<std::pair<std::string, type>>& required_arguments = func->get_arguments();
		const std::vector<node_ptr>& given_arguments = node.get_function_arguments();

		// check if the argument counts match
		if(!func->is_variadic() && required_arguments.size() != given_arguments.size()) {
			return outcome::failure(
				error::emit<error_code::function_argument_count_mismatch>(
					file_range{}, //node.get_declared_position(),
					node.get_function_identifier()
				)
			); // return on failure
		}

		// create LLVM IR for the argument values 
		std::vector<llvm::Value*> argument_values(required_arguments.size());
		for (u64 i = 0; i < required_arguments.size(); i++) {
			// get the argument value
			OUTCOME_TRY(auto argument_result, given_arguments[i]->accept(
				*this,
				code_generation_context(required_arguments[i].second)
			));

			// cast the given argument to match the required argument's type, if necessary
			argument_values[i] = cast_value(
				argument_result, 
				required_arguments[i].second, 
				given_arguments[i]->get_declared_range()
			);
		}

		// parse variadic arguments
		for (u64 i = required_arguments.size(); i < given_arguments.size(); i++) {
			// get the argument value
			OUTCOME_TRY(auto argument_value, given_arguments[i]->accept(
				*this,
				{}
			));

			const type argument_type = argument_value->get_type();

			// default variadic promotions:
			if(argument_type == type(type::base::f32, 0)) {
				// f32 -> f64
				llvm::Value* argument_value_cast = cast_value(
					argument_value,
					type(type::base::f64, 0),
					given_arguments[i]->get_declared_range()
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
					given_arguments[i]->get_declared_range()
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
		llvm::CallInst* call_inst = m_context->get_builder().CreateCall(
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