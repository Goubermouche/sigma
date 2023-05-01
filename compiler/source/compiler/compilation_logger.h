#pragma once
#include "../codegen/llvm_wrappers/type.h"

namespace channel{
	class compilation_logger {
	public:
		// IR
		// warnings
		static void emit_cast_warning(u64 line_number, type original_type, type target_type);
		static void emit_function_return_type_cast_warning(u64 line_number, type original_type, type target_type);
		static void emit_function_return_auto_generate_warning(u64 line_number, const std::string& function_name);

		// errors
		static void emit_unexpected_token_error(u64 line_number, token expected_token, token received_token);
		static void emit_unhandled_token_error(u64 line_number, token received_token);
		static void emit_unhandled_number_format_error(u64 line_number, type received_type);
		static void emit_token_is_not_type_error(u64 line_number, token received_token);
		static void emit_main_entry_point_missing_error();
		static void emit_main_entry_point_has_to_be_i32(type received_type);

		static void emit_variable_not_found_error(u64 line_number, const std::string& variable_name);
		static void emit_local_variable_already_defined_error(u64 line_number, const std::string& variable_name);
		static void emit_local_variable_already_defined_in_global_scope_error(u64 line_number, const std::string& variable_name);
		static void emit_global_variable_already_defined_error(u64 line_number, const std::string& variable_name);
		static void emit_cannot_allocate_for_non_pointer_type_error(u64 line_number, type ty);
		static void emit_cannot_cast_pointer_type_error(u64 line_number, type ty1, type ty2);
		static void emit_array_access_on_non_pointer_error(u64 line_number, type actual_type, const std::string& variable_name);
		static void emit_cannot_apply_unary_function_to_non_identifier_error(u64 line_number);

		static void emit_function_not_found_error(u64 line_number, const std::string& function_name);
		static void emit_function_already_defined_error(u64 line_number, const std::string& function_name);
		static void emit_function_argument_missing_error(u64 line_number);
		static void emit_function_argument_count_mismatch_error(u64 line_number, const std::string& function_name);
		static void emit_function_argument_type_mismatch_error(u64 line_number, u64 argument_index, type expected_type, type received_type, const std::string& function_name);

		static void emit_conjunction_operation_expects_booleans(u64 line_number, type left, type right);
		static void emit_disjunction_operation_expects_booleans(u64 line_number, type left, type right);
		static void emit_for_conditional_has_to_be_boolean(u64 line_number, type actual_type);
		static void emit_break_outside_loop_error(u64 line_number);

		static void emit_unary_operation_expects_numerical(u64 line_number, type actual_type);

		static void emit_cannot_open_file_error(const std::string& filepath);
		// lexer
		// errors
		static void emit_invalid_dot_character_at_token_start_error();
		static void emit_invalid_double_underscore_error();
		static void emit_invalid_number_format_only_one_dot_allowed_error();
		static void emit_invalid_number_format_unsigned_number_may_not_contain_dot_characters_error();
		static void emit_invalid_number_format_floating_point_must_contain_dot_character_error();
		static void emit_unterminated_character_literal_error();
		static void emit_unterminated_string_literal_error();

		// compilation
		// errors
		static void emit_delete_file_failed_error(const std::string& filepath);
	private:
		static void emit_warning(u64 line_number, const std::string& message);
		static void emit_error(u64 line_number, const std::string& message);
		static void emit_error(const std::string& message);
	};
}