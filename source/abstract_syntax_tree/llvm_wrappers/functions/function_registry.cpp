#include "function_registry.h"
#include "abstract_syntax_tree/abstract_syntax_tree_context.h"

namespace sigma {
	function_ptr function_registry::get_function(
		const std::string& identifier,
		const ptr<abstract_syntax_tree_context>& context
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

	function_declaration_ptr function_registry::get_external_function_declaration(
		const std::string& identifier
	) {
		// try and find an external function declaration
		const auto it = g_external_function_declarations.find(identifier);
		if (it != g_external_function_declarations.end()) {
			return it->second;
		}

		return nullptr;
	}

	utility::outcome::result<void> function_registry::concatenate(
		const function_registry& other
	) {
		for(const auto& function : other.m_functions) {
			if(m_functions.contains(function.first)) {
				// return outcome::failure(
				// 	error::emit<error_code::function_already_defined_at>(
				// 		file_range{}, //function.second->get_position(),
				// 		function.first,
				// 		m_functions[function.first]->get_position()
				// 	)
				// );
			}

			m_functions.insert(function);
		}

		return utility::outcome::success();
	}

	bool function_registry::insert_function(
		const std::string& identifier,
		function_ptr function
	) {
		return m_functions.insert({ identifier , function }).second;
	}

	bool function_registry::contains_function(
		const std::string& identifier
	) const {
		return m_functions.contains(identifier) || 
			g_external_function_declarations.contains(identifier);
		// || m_function_declarations ? 
	}
}