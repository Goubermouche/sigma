#include "codegen_visitor.h"
#include <llvm/IR/Verifier.h>

#include "codegen/abstract_syntax_tree/translation_unit_node.h"

namespace channel {
	codegen_visitor::codegen_visitor(const parser& parser)
		: m_parser(parser), m_scope(new scope(nullptr, nullptr)), m_function_registry(parser.get_function_registry()), m_builder(m_context) {
		m_module = std::make_unique<llvm::Module>("channel", m_context);

		// printf
		{
			const std::vector<llvm::Type*> arg_types = { llvm::Type::getInt8PtrTy(m_context) };
			llvm::FunctionType* func_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(m_context), arg_types, true);
			llvm::Function* func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "printf", m_module.get());

			m_function_registry.insert_function("print", std::make_shared<function>(
				type(type::base::i32, 0),
				func,
				std::vector<std::pair<std::string, type>>{
					{ "print", type(type::base::character, 1) }
				},
				true
			));
		}

		// putchar
		{
			const std::vector<llvm::Type*> arg_types = { llvm::Type::getInt8Ty(m_context) };
			llvm::FunctionType* func_type = llvm::FunctionType::get(llvm::Type::getVoidTy(m_context), arg_types, false);
			llvm::Function* func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "putchar", m_module.get());

			m_function_registry.insert_function("printc", std::make_shared<function>(
				type(type::base::empty, 0),
				func,
				std::vector<std::pair<std::string, type>>{
					{ "character", type(type::base::character, 0) }
				},
				false
			));
		}

		// malloc
		{
			const std::vector<llvm::Type*> arg_types = { llvm::Type::getInt64Ty(m_context) };
			llvm::FunctionType* func_type = llvm::FunctionType::get(llvm::Type::getInt8PtrTy(m_context), arg_types, false);
			llvm::Function* func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "malloc", m_module.get());

			m_function_registry.insert_function("malloc", std::make_shared<function>(
				type(type::base::i8, 1),
				func,
				std::vector<std::pair<std::string, type>>{
					{ "size", type(type::base::u64, 0) }
				},
				false
			));
		}

		// memset
		{
			const std::vector<llvm::Type*> arg_types = {
				llvm::Type::getInt8PtrTy(m_context), // void *ptr
				llvm::Type::getInt32Ty(m_context),   // int value
				llvm::Type::getInt64Ty(m_context)    // size_t num (assuming 64-bit target)
			};

			llvm::FunctionType* func_type = llvm::FunctionType::get(llvm::Type::getInt8PtrTy(m_context), arg_types, false);
			llvm::Function* func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "memset", m_module.get());

			m_function_registry.insert_function("memset", std::make_shared<function>(
				type(type::base::empty, 1),
				func,
				std::vector<std::pair<std::string, type>>{
					{ "a", type(type::base::character, 1)},
					{ "b", type(type::base::i32, 0) },
					{ "c", type(type::base::u64, 0) }
				},
				false
			));
		}

		// sin
		{
			const std::vector<llvm::Type*> arg_types = {
				llvm::Type::getDoubleTy(m_context),
			};

			llvm::FunctionType* func_type = llvm::FunctionType::get(llvm::Type::getDoubleTy(m_context), arg_types, false);
			llvm::Function* func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "sin", m_module.get());

			m_function_registry.insert_function("sin", std::make_shared<function>(
				type(type::base::f64, 0),
				func,
				std::vector<std::pair<std::string, type>>{
					{ "size", type(type::base::f64, 0)}
				},
				false
			));
		}


		// cos
		{
			const std::vector<llvm::Type*> arg_types = {
				llvm::Type::getDoubleTy(m_context),
			};

			llvm::FunctionType* func_type = llvm::FunctionType::get(llvm::Type::getDoubleTy(m_context), arg_types, false);
			llvm::Function* func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, "cos", m_module.get());

			m_function_registry.insert_function("cos", std::make_shared<function>(
				type(type::base::f64, 0),
				func,
				std::vector<std::pair<std::string, type>>{
					{ "size", type(type::base::f64, 0)}
				},
				false
			));
		}
	}

	bool codegen_visitor::generate() {
		// walk the abstract syntax tree
		value_ptr tmp_value;
		abstract_syntax_tree tree = m_parser.get_abstract_syntax_tree();

		for (node* n : tree) {
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
		// check if we have a valid 'main' function
		if(!verify_main_entry_point()) {
			return false;
		}

		// check for IR errors
		if (llvm::verifyModule(*m_module, &llvm::outs())) {
			return false;
		}

		return true;
	}

	bool codegen_visitor::visit_translation_unit_node(translation_unit_node& node, value_ptr& out_value) {
		value_ptr temp_value;
		for(const auto& n : node.get_nodes()) {
			if(!n->accept(*this, temp_value)) {
				return false;
			}
		}

		return true;
	}

	bool codegen_visitor::verify_main_entry_point() const {
		// check if we have a main entry point
		if(!m_function_registry.contains_function("main")) {
			error::emit<4012>().print();
			return false;
		}

		// check if the main entry point's return type is an i32
		const function_ptr func = m_function_registry.get_function("main");
		if(func->get_return_type().get_base() != type::base::i32) {
			error::emit<4013>(func->get_return_type()).print();
			return false;
		}

		return true;
	}

	bool codegen_visitor::cast_value(llvm::Value*& out_value, const value_ptr source_value, type target_type, const token_position& position) {
		// both types are the same
		if (source_value->get_type() == target_type) {
			out_value = source_value->get_value();
			return true;
		}

		// cast function call
		if (source_value->get_type() == type(type::base::function_call, 0)) {
			// use the function return type as its type
			const type function_return_type = m_function_registry.get_function(source_value->get_name())->get_return_type();

			// both types are the same 
			if (function_return_type == target_type) {
				out_value = source_value->get_value();
				return true;
			}

			// don't allow pointer casting for now
			//if (function_return_type.is_pointer() || target_type.is_pointer()) {
			//	compilation_logger::emit_cannot_cast_pointer_type_error(line_number, source_value->get_type(), target_type);
			//	return false;
			//}

			warning::emit<3001>(position, function_return_type, target_type).print();

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

		// don't allow pointer casting for now
		//if (source_value->get_type().is_pointer() || target_type.is_pointer()) {
		//	compilation_logger::emit_cannot_cast_pointer_type_error(line_number, source_value->get_type(), target_type);
		//	return false;
		//}

		warning::emit<3002>(position, source_value->get_type(), target_type).print();

		// get the LLVM value and type for source and target
		llvm::Value* source_llvm_value = source_value->get_value();
		llvm::Type* target_llvm_type = target_type.get_llvm_type(source_llvm_value->getContext());

		// bool to i32
		if (source_value->get_type().get_base() == type::base::boolean && target_type.get_base() == type::base::i32) {
			out_value = m_builder.CreateZExt(source_llvm_value, target_llvm_type, "zext");
			return true;
		}

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

	bool codegen_visitor::get_named_value(value_ptr& out_value, const std::string& variable_name) {
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
