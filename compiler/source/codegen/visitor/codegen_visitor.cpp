#include "codegen_visitor.h"
#include <llvm/IR/Verifier.h>

#include "codegen/abstract_syntax_tree/translation_unit_node.h"

namespace channel {
	codegen_visitor::codegen_visitor(
		const parser& parser
	) : m_parser(parser),
	m_scope(new scope(nullptr, nullptr)),
	m_builder(m_context),
	m_module(std::make_unique<llvm::Module>("channel", m_context)) {
		// initialize function declarations
		for(const auto& [function_identifier, function_declaration] : m_function_registry.get_external_function_declarations()) {
			const std::vector<std::pair<std::string, type>>& arguments = function_declaration->get_arguments();
			std::vector<llvm::Type*> argument_types(arguments.size());

			// initialize argument types
			for (u64 i = 0; i < arguments.size(); i++) {
				argument_types[i] = arguments[i].second.get_llvm_type(m_context);
			}

			llvm::FunctionType* function_type = llvm::FunctionType::get(
				function_declaration->get_return_type().get_llvm_type(m_context),
				argument_types,
				function_declaration->is_variadic()
			);

			llvm::Function* func = llvm::Function::Create(
				function_type,
				llvm::Function::ExternalLinkage,
				function_declaration->get_external_function_name(),
				m_module.get()
			);

			// insert the function declaration and treat it like a regular function
			m_function_registry.insert_function(
				function_identifier,
				std::make_shared<function>(
					function_declaration->get_return_type(),
					func,
					arguments,
					function_declaration->is_variadic()
				)
			);
		}
	}

	bool codegen_visitor::generate() {
		abstract_syntax_tree tree = m_parser.get_abstract_syntax_tree();

		// walk the abstract syntax tree
		for (node* n : tree) {
			acceptation_result result = n->accept(*this, {});

			if(!result.has_value()) {
				result.error().print();
				return false; // return on failure
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
		console::out << color::red;
		if (llvm::verifyModule(*m_module, &llvm::outs())) {
			console::out << color::white;
			return false;
		}
		console::out << color::white;

		return true;
	}

	acceptation_result codegen_visitor::visit_translation_unit_node(translation_unit_node& node, const codegen_context& context) {
		for(const auto& n : node.get_nodes()) {
			acceptation_result result = n->accept(*this, context);
			if(!result.value()) {
				return result; // return on failure
			}
		}

		return nullptr;
	}

	bool codegen_visitor::verify_main_entry_point() const {
		// check if we have a main entry point
		if(!m_function_registry.contains_function("main")) {
			error::emit<4012>().print();
			return false; // return on failure
		}

		// check if the main entry point's return type is an i32
		const function_ptr func = m_function_registry.get_function("main");
		if(func->get_return_type().get_base() != type::base::i32) {
			error::emit<4013>(func->get_return_type()).print();
			return false; // return on failure
		}

		return true;
	}

	llvm::Value* codegen_visitor::cast_value(const value_ptr& source_value, type target_type, const token_position& position) {
		// both types are the same
		if (source_value->get_type() == target_type) {
			return source_value->get_value();
		}

		// cast a function call
		if (source_value->get_type() == type(type::base::function_call, 0)) {
			// use the function return type as its type
			const type function_return_type = m_function_registry.get_function(source_value->get_name())->get_return_type();

			// both types are the same 
			if (function_return_type == target_type) {
				return source_value->get_value();
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
					return m_builder.CreateFPToSI(function_call_result, target_llvm_type, "fptosi");
				}

				if (target_type.is_floating_point()) {
					return m_builder.CreateFPCast(function_call_result, target_llvm_type, "fpcast");
				}
			}
			else if (function_return_type.is_integral()) {
				if (target_type.is_floating_point()) {
					return m_builder.CreateSIToFP(function_call_result, target_llvm_type, "sitofp");
				}

				if (target_type.is_integral()) {
					return m_builder.CreateIntCast(function_call_result, target_llvm_type, target_type.is_signed(), "intcast");
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

		// boolean to i32
		if (source_value->get_type().get_base() == type::base::boolean && target_type.get_base() == type::base::i32) {
			return m_builder.CreateZExt(source_llvm_value, target_llvm_type, "zext");
		}

		// floating-point to integer
		if (source_value->get_type().is_floating_point() && target_type.is_integral()) {
			return m_builder.CreateFPToSI(source_llvm_value, target_llvm_type);
		}

		// integer to floating-point
		if (source_value->get_type().is_integral() && target_type.is_floating_point()) {
			return m_builder.CreateSIToFP(source_llvm_value, target_llvm_type);
		}

		// floating-point upcast or downcast
		if (source_value->get_type().is_floating_point() && target_type.is_floating_point()) {
			return m_builder.CreateFPCast(source_llvm_value, target_llvm_type, "fpcast");
		}

		// other cases
		if (source_value->get_type().get_bit_width() < target_type.get_bit_width()) {
			// perform upcast
			if (source_value->get_type().is_unsigned()) {
				return m_builder.CreateZExt(source_llvm_value, target_llvm_type, "zext");
			}

			return m_builder.CreateSExt(source_llvm_value, target_llvm_type, "sext");
		}

		// downcast
		return m_builder.CreateTrunc(source_llvm_value, target_llvm_type, "trunc");
	}

	bool codegen_visitor::get_named_value(value_ptr& out_value, const std::string& variable_name) {
		// check the local scope
		out_value = m_scope->get_named_value(variable_name);

		if (!out_value) {
			// variable with the given name was not found in the local scope hierarchy, check global variables
			out_value = m_global_named_values[variable_name];

			if (!out_value) {
				return false; // variable not found
			}
		}

		return true;
	}
}
