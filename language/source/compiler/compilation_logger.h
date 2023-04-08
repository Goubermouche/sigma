#pragma once
#include "../utility/console.h"
#include "../type.h"

namespace channel{
	class compilation_logger {
	public:
		// warnings
		static void emit_cast_warning(u64 line_number, type original_type, type target_type);
		static void emit_function_return_type_cast_warning(u64 line_number, type original_type, type target_type);
		static void emit_function_return_auto_generate_warning(u64 line_number, const std::string& function_name);

		// errors
		static void emit_unexpected_token_error(u64 line_number, token expected_token, token received_token);
		static void emit_unhandled_token_error(u64 line_number, token received_token);
		static void emit_unhandled_number_format_error(u64 line_number, token received_token);
		static void emit_main_entry_point_missing_error();

		static void emit_variable_not_found_error(u64 line_number, const std::string& variable_name);

		static void emit_local_variable_already_defined_error(u64 line_number, const std::string& variable_name);
		static void emit_local_variable_already_defined_in_global_scope_error(u64 line_number, const std::string& variable_name);

		static void emit_global_variable_already_defined_error(u64 line_number, const std::string& variable_name);

		static void emit_function_not_found_error(u64 line_number, const std::string& function_name);
		static void emit_function_already_defined_error(u64 line_number, const std::string& function_name);
	private:
		static void emit_warning(u64 line_number, const std::string& message);
		static void emit_error(u64 line_number, const std::string& message);
	};
}

