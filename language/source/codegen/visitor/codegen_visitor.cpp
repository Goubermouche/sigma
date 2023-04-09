#include "codegen_visitor.h"
#include <llvm/IR/Verifier.h>

namespace channel {
	codegen_visitor::codegen_visitor()
		: m_scope(new scope(nullptr)), m_builder(m_context) {
		m_module = std::make_unique<llvm::Module>("channel", m_context);
	}

	bool codegen_visitor::generate(parser& parser) {
		// parse the source file
		std::vector<node*> abstract_syntax_tree;
		if (!parser.parse(abstract_syntax_tree)) {
			return false; // parsing failed, return false
		}

		// walk the abstract syntax tree
		value* tmp_value;
		for (node* n : abstract_syntax_tree) {
			if(!n->accept(*this, tmp_value)) {
				return false;
			}
		}

		initialize_global_variables();
		std::cout << "----------------------------\n";
		return true;
	}

	void codegen_visitor::initialize_global_variables() {
		// create the global ctors array
		llvm::ArrayType* updated_ctor_array_type = llvm::ArrayType::get(CTOR_STRUCT_TYPE, m_ctors.size());
		llvm::Constant* updated_ctors = llvm::ConstantArray::get(updated_ctor_array_type, m_ctors);
		new llvm::GlobalVariable(*m_module, updated_ctor_array_type, false, llvm::GlobalValue::AppendingLinkage, updated_ctors, "llvm.global_ctors");
	}

	void codegen_visitor::print_intermediate_representation() const {
		m_module->print(llvm::outs(), nullptr);
	}

	bool codegen_visitor::verify_intermediate_representation() const {
		// check if we have a 'main' function
		if(!has_main_entry_point()) {
			compilation_logger::emit_main_entry_point_missing_error();
		}

		// check for IR errors
		if (llvm::verifyModule(*m_module, &llvm::outs())) {
			return false;
		}

		return true;
	}

	bool codegen_visitor::has_main_entry_point() const {
		return m_functions.at("main") != nullptr;
	}

	llvm::Value* codegen_visitor::cast_value(const value* source_value, type target_type, u64 line_number) {
		// both types are the same 
		if(source_value->get_type() == target_type) {
			return source_value->get_value();
		}

		// cast function call
		if (source_value->get_type() == type::function_call) {
			// use the function return type as its type
			const type function_return_type = m_functions[source_value->get_name()]->get_return_type();

			// both types are the same 
			if(function_return_type == target_type) {
				return source_value->get_value();
			}

			compilation_logger::emit_function_return_type_cast_warning(line_number, function_return_type, target_type);

			llvm::Value* function_call_result = source_value->get_value();
			llvm::Type* target_llvm_type = type_to_llvm_type(target_type, function_call_result->getContext());

			// perform the cast op
			if (is_type_floating_point(function_return_type)) {
				if (is_type_integral(target_type)) {
					return m_builder.CreateFPToSI(function_call_result, target_llvm_type, "fptosi");
				}

				if (is_type_floating_point(target_type)) {
					return m_builder.CreateFPCast(function_call_result, target_llvm_type, "fpcast");
				}
			}
			else if (is_type_integral(function_return_type)) {
				if (is_type_floating_point(target_type)) {
					return m_builder.CreateSIToFP(function_call_result, target_llvm_type, "sitofp");
				}

				if (is_type_integral(target_type)) {
					return m_builder.CreateIntCast(function_call_result, target_llvm_type, is_type_signed(target_type), "intcast");
				}
			}
		}

		compilation_logger::emit_cast_warning(line_number, source_value->get_type(), target_type);

		// get the LLVM value and type for source and target
		llvm::Value* source_llvm_value = source_value->get_value();
		llvm::Type* target_llvm_type = type_to_llvm_type(target_type, source_llvm_value->getContext());

		// floating-point to integer
		if (is_type_floating_point(source_value->get_type()) && is_type_integral(target_type)) {
			return m_builder.CreateFPToSI(source_llvm_value, target_llvm_type);
		}

		// integer to floating-point
		if (is_type_integral(source_value->get_type()) && is_type_floating_point(target_type)) {
			return m_builder.CreateSIToFP(source_llvm_value, target_llvm_type);
		}

		// other cases
		if (get_type_bit_width(source_value->get_type()) < get_type_bit_width(target_type)) {
			// perform upcast
			if (is_type_unsigned(source_value->get_type())) {
				return m_builder.CreateZExt(source_llvm_value, target_llvm_type, "zext");
			}

			return m_builder.CreateSExt(source_llvm_value, target_llvm_type, "sext");
		}

		// downcast
		return m_builder.CreateTrunc(source_llvm_value, target_llvm_type, "trunc");
	}
}