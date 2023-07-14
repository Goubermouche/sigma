#include "function_registry.h"
#include "code_generator/code_generator_context.h"

namespace sigma {
	function_ptr function_registry::get_function(
		const std::string& identifier,
		const std::shared_ptr<code_generator_context>& context
	) {
		// locate a defined function
		const auto it = m_functions.find(identifier);
		if(it != m_functions.end()) {
			return it->second;
		}

		// function "does not exist", try and declare an external function
		const function_declaration_ptr external_function_decl = get_external_function_declaration(
			identifier
		);

		// no function with the given identifier was found
		if (!external_function_decl) {
			return nullptr;
		}

		// initialize the function declaration
		const std::vector<std::pair<std::string, type>>& arguments = external_function_decl->get_arguments();
		std::vector<llvm::Type*> argument_types(arguments.size());

		// initialize argument types
		for (u64 i = 0; i < arguments.size(); i++) {
			argument_types[i] = arguments[i].second.get_llvm_type(
				context->get_context()
			);
		}

		llvm::FunctionType* function_type = llvm::FunctionType::get(
			external_function_decl->get_return_type().get_llvm_type(context->get_context()),
			argument_types,
			external_function_decl->is_variadic()
		);

		llvm::Function* function_block = llvm::Function::Create(
			function_type,
			llvm::Function::ExternalLinkage,
			external_function_decl->get_external_function_name(),
			context->get_module().get()
		);

		// insert the function declaration and treat it like a regular function
		function_ptr func = std::make_shared<function>(
			external_function_decl->get_return_type(),
			function_block,
			arguments,
			external_function_decl->is_variadic()
		);

		insert_function(
			identifier,
			func
		);

		return func;
	}

	function_declaration_ptr function_registry::get_function_declaration(
		const std::string& identifier
	) const {
		// try and find a regular declaration
		const auto it = m_function_declarations.find(identifier);
		if (it != m_function_declarations.end()) {
			return it->second;
		}

		return nullptr;
	}

	function_declaration_ptr function_registry::get_external_function_declaration(
		const std::string& identifier
	) const 	{
		// try and find an external function declaration
		const auto it = m_external_function_declarations.find(identifier);
		if (it != m_external_function_declarations.end()) {
			return it->second;
		}

		return nullptr;
	}

	const std::unordered_map<std::string, function_declaration_ptr>& function_registry::get_external_function_declarations() const {
		return m_external_function_declarations;
	}

	void function_registry::insert_function(
		const std::string& identifier,
		function_ptr function
	) {
		m_functions[identifier] = function;
	}

	void function_registry::insert_function_declaration(
		const std::string& identifier, 
		function_declaration_ptr function
	) {
		m_function_declarations[identifier] = function;
	}

	bool function_registry::contains_function(
		const std::string& identifier
	) const {
		return m_functions.contains(identifier) || 
			m_external_function_declarations.contains(identifier);
	}

	bool function_registry::contains_function_declaration(
		const std::string& identifier
	) const {
		return m_function_declarations.contains(identifier) || 
			m_external_function_declarations.contains(identifier);
	}
}