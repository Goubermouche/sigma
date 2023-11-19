#include "abstract_syntax_tree_visitor.h"
#include <llvm/IR/Verifier.h>

namespace sigma {
	abstract_syntax_tree_visitor::abstract_syntax_tree_visitor(
		const s_ptr<abstract_syntax_tree>& abstract_syntax_tree
	) : m_abstract_syntax_tree(abstract_syntax_tree),
	m_context(std::make_shared<abstract_syntax_tree_context>()) {}

	utility::outcome::result<s_ptr<abstract_syntax_tree_context>> abstract_syntax_tree_visitor::generate() {
		// walk the abstract syntax tree
		for (const node_ptr node : *m_abstract_syntax_tree) {
			OUTCOME_TRY(node->accept(*this, {}));
		}

		return m_context;
	}

	llvm::Value* abstract_syntax_tree_visitor::cast_value(
		const value_ptr& source_value,
		type target_type, 
		const utility::file_range& range
	) const {
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

			utility::console::out << *utility::warning::emit_assembly<utility::warning_code::implicit_function_type_cast>(
				range,
				function_return_type,
				target_type
			);

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

		utility::console::out << *utility::warning::emit_assembly<utility::warning_code::implicit_type_cast>(
			range,
			source_value->get_type(),
			target_type
		);

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
