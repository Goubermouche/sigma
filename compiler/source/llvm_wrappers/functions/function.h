#pragma once
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
		 * \param position (optional) Position the function declaration was declared at
		 */
		function_declaration(
			type return_type,
			const std::vector<std::pair<std::string, type>>& arguments,
			bool is_variadic,
			const std::string& external_function_name = "",
			const file_position& position = {}
		);

		type get_return_type() const;
		const std::vector<std::pair<std::string, type>>& get_arguments() const;
		bool is_variadic() const;
		const std::string& get_external_function_name() const;
		const file_position& get_position() const;
	protected:
		type m_return_type;
		std::vector<std::pair<std::string, type>> m_arguments;
		bool m_is_variadic;
		std::string m_external_function_name; // function name which refers to a relevant C function name
		file_position m_position;
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
		 * \param position (optional) Position the function was declared at
		 */
		function(
			type return_type, 
			llvm::Function* function,
			const std::vector<std::pair<std::string, type>>& arguments,
			bool is_variadic,
			const file_position& position = {}
		);

		llvm::Function* get_function() const;
	private:
		llvm::Function* m_value;
	};
}
