#include "codegen_visitor.h"
#include <llvm/IR/Verifier.h>

namespace channel {
	codegen_visitor::codegen_visitor()
		: m_scope(new scope(nullptr)), m_builder(m_context) {
		m_module = std::make_unique<llvm::Module>("channel", m_context);

		llvm::PointerType* printf_arg_type = llvm::Type::getInt8PtrTy(m_context);
		std::vector<llvm::Type*> printf_arg_types = { printf_arg_type };
		llvm::FunctionType* printf_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(m_context), printf_arg_types, true);
		llvm::Function* printf_func = llvm::Function::Create(printf_type, llvm::Function::ExternalLinkage, "printf", m_module.get());

		m_functions["print"] = new function(type(type::base::empty, 0), printf_func, {{"format", type(type::base::character, 1)}}, true);
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
		return true;
	}

	std::shared_ptr<llvm::Module> codegen_visitor::get_module() {
		return m_module;
	}

	void codegen_visitor::initialize_global_variables() {
		// create the global ctors array
		llvm::ArrayType* updated_ctor_array_type = llvm::ArrayType::get(CTOR_STRUCT_TYPE, m_global_ctors.size());
		llvm::Constant* updated_ctors = llvm::ConstantArray::get(updated_ctor_array_type, m_global_ctors);
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

	bool codegen_visitor::cast_value(llvm::Value*& out_value, const value* source_value, type target_type, u64 line_number) {
		// both types are the same 
		if(source_value->get_type() == target_type) {
			out_value = source_value->get_value();
			return true;
		}

		// don't allow pointer casting for now
		if(source_value->get_type().is_pointer() || target_type.is_pointer()) {
			compilation_logger::emit_cannot_cast_pointer_type_error(line_number, source_value->get_type(), target_type);
			return false;
		}

		// cast function call
		if (source_value->get_type() == type(type::base::function_call, 0)) {
			// use the function return type as its type
			const type function_return_type = m_functions[source_value->get_name()]->get_return_type();

			// both types are the same 
			if(function_return_type == target_type) {
				out_value = source_value->get_value();
				return true;
			}

			compilation_logger::emit_function_return_type_cast_warning(line_number, function_return_type, target_type);

			llvm::Value* function_call_result = source_value->get_value();
			llvm::Type* target_llvm_type = target_type.get_llvm_type(function_call_result->getContext());

			// perform the cast op
			if (function_return_type.is_floating_point()) {
				if (target_type.is_integral()) {
					out_value = m_builder.CreateFPToSI(function_call_result, target_llvm_type, "fptosi");
					return true;
				}

				if (target_type.is_floating_point()) {
					out_value = m_builder.CreateFPCast(function_call_result, target_llvm_type, "fpcast");
					return true;
				}
			}
			else if (function_return_type.is_integral()) {
				if (target_type.is_floating_point()) {
					out_value = m_builder.CreateSIToFP(function_call_result, target_llvm_type, "sitofp");
					return true;
				}

				if (target_type.is_integral()) {
					out_value = m_builder.CreateIntCast(function_call_result, target_llvm_type, target_type.is_signed(), "intcast");
					return true;
				}
			}
		}

		compilation_logger::emit_cast_warning(line_number, source_value->get_type(), target_type);

		// get the LLVM value and type for source and target
		llvm::Value* source_llvm_value = source_value->get_value();
		llvm::Type* target_llvm_type = target_type.get_llvm_type(source_llvm_value->getContext());

		// floating-point to integer
		if (source_value->get_type().is_floating_point() && target_type.is_integral()) {
			out_value = m_builder.CreateFPToSI(source_llvm_value, target_llvm_type);
			return true;
		}

		// integer to floating-point
		if (source_value->get_type().is_integral() && target_type.is_floating_point()) {
			out_value = m_builder.CreateSIToFP(source_llvm_value, target_llvm_type);
			return true;
		}

		// floating-point upcast or downcast
		if (source_value->get_type().is_floating_point() && target_type.is_floating_point()) {
			std::cout << "hewe\n";
			out_value = m_builder.CreateFPCast(source_llvm_value, target_llvm_type, "fpcast");
			return true;
		}

		// other cases
		if (source_value->get_type().get_bit_width() < target_type.get_bit_width()) {
			// perform upcast
			if (source_value->get_type().is_unsigned()) {
				out_value = m_builder.CreateZExt(source_llvm_value, target_llvm_type, "zext");
				return true;
			}

			out_value = m_builder.CreateSExt(source_llvm_value, target_llvm_type, "sext");
			return true;
		}

		// downcast
		out_value = m_builder.CreateTrunc(source_llvm_value, target_llvm_type, "trunc");
		return true;
	}

	bool codegen_visitor::get_named_value(value*& out_value, const std::string& variable_name) {
		// check the local scope
		out_value = m_scope->get_named_value(variable_name);
		if (!out_value) {
			// variable with the given name was not found in the local scope hierarchy, check global variables
			out_value = m_global_named_values[variable_name];

			if (!out_value) {
				// variable not found
				return false;
			}
		}

		return true;
	}
}