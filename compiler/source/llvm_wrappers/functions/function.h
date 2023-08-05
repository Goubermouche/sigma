#pragma once
#include "utility/filesystem/filesystem.h"
#include "llvm_wrappers/type.h"

namespace sigma {
	/**
	 * \brief Base function declaration class, contains information about a specific function declaration. 
	 */
	class function_declaration {
	public:
		/**
		 * \brief Constructs a new function declaration. 
		 * \param return_type Return type of the function declaration
		 * \param arguments List of arguments the function declaration expects
		 * \param is_variadic Truthy value representing the variadic nature of the function declaration 
		 * \param external_function_name (optional) Name of the external function this function derives from
		 * \param range (optional) Range the original value takes up in the source file
		 */
		function_declaration(
			type return_type,
			const std::vector<std::pair<std::string, type>>& arguments,
			bool is_variadic,
			const std::string& external_function_name = "",
			const file_range& range = {}
		);

		type get_return_type() const;
		const std::vector<std::pair<std::string, type>>& get_arguments() const;
		bool is_variadic() const;
		const std::string& get_external_function_name() const;
		const file_range& get_range() const;
	protected:
		type m_return_type;
		std::vector<std::pair<std::string, type>> m_arguments;
		bool m_is_variadic;
		std::string m_external_function_name; // function name which refers to a relevant C function name
		file_range m_range;
	};

	/**
	 * \brief Base function definition class, derives from the function declaration, soft wrapper around
	 * llvm::Function*.
	 */
	class function : public function_declaration{
	public:
		/**
		 * \brief Constructs the function definition.
		 * \param return_type Return type of the function
		 * \param function llvm::Function* which refers to the llvm function object
		 * \param arguments List of arguments the function expects
		 * \param is_variadic Truthy value representing the variadic nature of the function 
		 * \param range (optional) Range the original value takes up in the source file
		 */
		function(
			type return_type, 
			llvm::Function* function,
			const std::vector<std::pair<std::string, type>>& arguments,
			bool is_variadic,
			const file_range& range = {}
		);

		llvm::Function* get_function() const;
	private:
		llvm::Function* m_value;
	};
}
