#pragma once
#include "llvm_wrappers/type.h"

namespace sigma {
	class function_declaration {
	public:
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

	class function : public function_declaration{
	public:
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
