#include "code_generator.h"
#include <llvm/IR/Verifier.h>

namespace sigma {
	code_generator::code_generator()
		: m_context(std::make_shared<code_generator_context>()) {}

	outcome::result<void> code_generator::generate() {
		// walk the abstract syntax tree
		for (node_ptr n : *m_abstract_syntax_tree) {
			OUTCOME_TRY(n->accept(*this, {}));
		}

		initialize_global_variables();
		initialize_used_external_functions();

		// verify the generated IR
		OUTCOME_TRY(verify_intermediate_representation());
		return outcome::success();
	}

	void code_generator::initialize_global_variables() const {
		// create the global ctors array
		llvm::ArrayType* updated_ctor_array_type = llvm::ArrayType::get(CTOR_STRUCT_TYPE, m_context->get_variable_registry().get_global_ctors_count());
		llvm::Constant* updated_ctors = llvm::ConstantArray::get(updated_ctor_array_type, m_context->get_variable_registry().get_global_ctors());
		new llvm::GlobalVariable(*m_context->get_module(), updated_ctor_array_type, false, llvm::GlobalValue::AppendingLinkage, updated_ctors, "llvm.global_ctors");
	}

	void code_generator::initialize_used_external_functions() const {}

	outcome::result<void> code_generator::verify_intermediate_representation() {
		// check if we have a valid 'main' function
		OUTCOME_TRY(verify_main_entry_point());
		
		// check for IR errors
		console::out << color::red;
		// if (llvm::verifyModule(*m_llvm_handler->get_module(), &llvm::outs())) {
		// 	console::out << color::white;
		// 	return error::emit<4016>();
		// }
		
		console::out << color::white;
		return outcome::success();
	}

	void code_generator::set_abstract_syntax_tree(
		std::shared_ptr<abstract_syntax_tree> abstract_syntax_tree
	) {
		m_abstract_syntax_tree = abstract_syntax_tree;
	}

	std::shared_ptr<code_generator_context> code_generator::get_llvm_context() {
		return m_context;
	}

	outcome::result<void> code_generator::verify_main_entry_point() {
		// check if we have a main entry point
		if(!m_context->get_function_registry().contains_function("main")) {
			return outcome::failure(error::emit<4012>()); // return on failure
		}

		// check if the main entry point's return type is an i32
		const function_ptr func = m_context->get_function_registry().get_function("main", m_context);
		if(func->get_return_type().get_base() != type::base::i32) {
			return outcome::failure(error::emit<4013>(func->get_return_type())); // return on failure
		}

		return outcome::success();
	}

	llvm::Value* code_generator::cast_value(const value_ptr& source_value, type target_type, const file_position& location) {
		// both types are the same
		if (source_value->get_type() == target_type) {
			return source_value->get_value();
		}

		// cast a function call
		if (source_value->get_type() == type(type::base::function_call, 0)) {
			// use the function return type as its type
			const type function_return_type = m_context->get_function_registry().get_function(
				source_value->get_name(),
				m_context
			)->get_return_type();

			// both types are the same 
			if (function_return_type == target_type) {
				return source_value->get_value();
			}

			// don't allow pointer casting for now
			//if (function_return_type.is_pointer() || target_type.is_pointer()) {
			//	compilation_logger::emit_cannot_cast_pointer_type_error(line_number, source_value->get_type(), target_type);
			//	return false;
			//}

			warning::emit<3001>(location, function_return_type, target_type)->print();

			llvm::Value* function_call_result = source_value->get_value();
			llvm::Type* target_llvm_type = target_type.get_llvm_type(function_call_result->getContext());

			// perform the cast op
			if (function_return_type.is_floating_point()) {
				if (target_type.is_integral()) {
					return m_context->get_builder().CreateFPToSI(function_call_result, target_llvm_type, "fptosi");
				}

				if (target_type.is_floating_point()) {
					return m_context->get_builder().CreateFPCast(function_call_result, target_llvm_type, "fpcast");
				}
			}
			else if (function_return_type.is_integral()) {
				if (target_type.is_floating_point()) {
					return m_context->get_builder().CreateSIToFP(function_call_result, target_llvm_type, "sitofp");
				}

				if (target_type.is_integral()) {
					return m_context->get_builder().CreateIntCast(function_call_result, target_llvm_type, target_type.is_signed(), "intcast");
				}
			}
		}

		// don't allow pointer casting for now
		//if (source_value->get_type().is_pointer() || target_type.is_pointer()) {
		//	compilation_logger::emit_cannot_cast_pointer_type_error(line_number, source_value->get_type(), target_type);
		//	return false;
		//}

		warning::emit<3002>(location, source_value->get_type(), target_type)->print();

		// get the LLVM value and type for source and target
		llvm::Value* source_llvm_value = source_value->get_value();
		llvm::Type* target_llvm_type = target_type.get_llvm_type(source_llvm_value->getContext());

		// boolean to i32
		if (source_value->get_type().get_base() == type::base::boolean && target_type.get_base() == type::base::i32) {
			return m_context->get_builder().CreateZExt(source_llvm_value, target_llvm_type, "zext");
		}

		// floating-point to integer
		if (source_value->get_type().is_floating_point() && target_type.is_integral()) {
			return m_context->get_builder().CreateFPToSI(source_llvm_value, target_llvm_type);
		}

		// integer to floating-point
		if (source_value->get_type().is_integral() && target_type.is_floating_point()) {
			return m_context->get_builder().CreateSIToFP(source_llvm_value, target_llvm_type);
		}

		// floating-point upcast or downcast
		if (source_value->get_type().is_floating_point() && target_type.is_floating_point()) {
			return m_context->get_builder().CreateFPCast(source_llvm_value, target_llvm_type, "fpcast");
		}

		// other cases
		if (source_value->get_type().get_bit_width() < target_type.get_bit_width()) {
			// perform upcast
			if (source_value->get_type().is_unsigned()) {
				return m_context->get_builder().CreateZExt(source_llvm_value, target_llvm_type, "zext");
			}

			return m_context->get_builder().CreateSExt(source_llvm_value, target_llvm_type, "sext");
		}

		// downcast
		return m_context->get_builder().CreateTrunc(source_llvm_value, target_llvm_type, "trunc");
	}
}
