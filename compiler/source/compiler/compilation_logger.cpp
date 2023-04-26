#include "compilation_logger.h"

namespace channel {
	void compilation_logger::emit_cast_warning(u64 line_number, type original_type, type target_type) {
		emit_warning(
			line_number,
			"implicit type cast ('" + original_type.to_string() + "' to '" + target_type.to_string() + "')"
		);
	}

	void compilation_logger::emit_function_return_type_cast_warning(u64 line_number, type original_type, type target_type) {
		emit_warning(
			line_number,
			"implicit function return type cast ('" + original_type.to_string() + "' to '" + target_type.to_string() + "')"
		);
	}

	void compilation_logger::emit_function_return_auto_generate_warning(u64 line_number, const std::string& function_name) {
		emit_warning(
			line_number,
			"implicit function return generated for function '" + function_name + "'"
		);
	}

	void compilation_logger::emit_unexpected_token_error(u64 line_number, token expected_token, token received_token) {
		emit_error(
			line_number,
			"unexpected token received (expected '" + token_to_string(expected_token) + "', but received '" + token_to_string(received_token) + "' instead)"
		);
	}

	void compilation_logger::emit_unhandled_token_error(u64 line_number, token received_token) {
		emit_error(
			line_number,
			"unhandled token received ('" + token_to_string(received_token) + "')"
		);
	}

	void compilation_logger::emit_unhandled_number_format_error(u64 line_number, type received_type) {
		emit_error(
			line_number,
			"unhandled non-numerical token received ('" + received_type.to_string() + "')"
		);
	}

	void compilation_logger::emit_token_is_not_type_error(u64 line_number, token received_token) {
		emit_error(
			line_number,
			"unexpected non-type token received ('" + token_to_string(received_token) + "')"
		);
	}

	void compilation_logger::emit_main_entry_point_missing_error() {
		emit_error(
			"unable to locate main entry point\n"
		);
	}

	void compilation_logger::emit_variable_not_found_error(u64 line_number, const std::string& variable_name)	{
		emit_error(
			line_number,
			"variable '" + variable_name + "' cannot be found"
		);
	}

	void compilation_logger::emit_local_variable_already_defined_error(u64 line_number, const std::string& variable_name) {
		emit_error(
			line_number,
			"local variable '" + variable_name + "' has already been defined before"
		);
	}

	void compilation_logger::emit_local_variable_already_defined_in_global_scope_error(u64 line_number, const std::string& variable_name) {
		emit_error(
			line_number,
			"local variable '" + variable_name + "' has already been defined in the global scope"
		);
	}

	void compilation_logger::emit_global_variable_already_defined_error(u64 line_number, const std::string& variable_name) {
		emit_error(
			line_number,
			"global variable '" + variable_name + "' has already been defined before"
		);
	}

	void compilation_logger::emit_cannot_allocate_for_non_pointer_type_error(u64 line_number, type ty) {
		emit_error(
			line_number,
			"cannot allocate memory for non-pointer type '" + ty.to_string() + "'"
		);
	}

	void compilation_logger::emit_cannot_cast_pointer_type_error(u64 line_number, type ty1, type ty2) {
		emit_error(
			line_number,
			"cannot cast between pointer types ('" + ty1.to_string() + "', '" + ty2.to_string() + "')"
		);
	}

	void compilation_logger::emit_array_access_on_non_pointer_error(u64 line_number, type actual_type, const std::string& variable_name) {
		emit_error(
			line_number,
			"invalid array access on non pointer type '" + actual_type.to_string() + "' ('" + variable_name + "')"
		);
	}

	void compilation_logger::emit_function_not_found_error(u64 line_number, const std::string& function_name) {
		emit_error(
			line_number,
			"function '" + function_name + "' cannot be found"
		);
	}

	void compilation_logger::emit_function_already_defined_error(u64 line_number, const std::string& function_name)	{
		emit_error(
			line_number,
			"function '" + function_name + "' has already been defined before"
		);
	}

	void compilation_logger::emit_function_argument_missing_error(u64 line_number) {
		emit_error(
			line_number,
			"function argument missing in function declaration"
		);
	}

	void compilation_logger::emit_function_argument_count_mismatch_error(u64 line_number, const std::string& function_name) {
		emit_error(
			line_number,
			"function argument count mismatch for function '" + function_name + "'"
		);
	}

	void compilation_logger::emit_function_argument_type_mismatch_error(u64 line_number, u64 argument_index, type expected_type, type received_type, const std::string& function_name) {
		emit_error(
			line_number,
			"function argument of function call to '"
			+ function_name
			+ "' at index '"
			+ std::to_string(argument_index) 
			+ "' resulted in a type mismatch (expected '"
			+ expected_type.to_string() + "', but got '"
			+ received_type.to_string() + "' instead)"
		);
	}

	void compilation_logger::emit_conjunction_operation_expects_booleans(u64 line_number, type left, type right) {
		emit_error(
			line_number,
			"conjunction operation expects two booleans, but received '" + left.to_string() + "' and '" + right.to_string() + "'"
		);
	}

	void compilation_logger::emit_disjunction_operation_expects_booleans(u64 line_number, type left, type right) {
		emit_error(
			line_number,
			"disjunction operation expects two booleans, but received '" + left.to_string() + "' and '" + right.to_string() + "'"
		);
	}

	void compilation_logger::emit_cannot_open_file_error(const std::string& filepath) {
		console::log(color::red, "[error:]: ");
		console::log(color::white, "cannot open file '" + filepath + "'\n");
	}

	void compilation_logger::emit_invalid_dot_character_at_token_start_error() {
		emit_error(
			"invalid '.' character detected at token start"
		);
	}

	void compilation_logger::emit_invalid_double_underscore_error() {
		emit_error(
			"two '_' characters immediately one after another are not allowed"
		);
	}

	void compilation_logger::emit_invalid_number_format_only_one_dot_allowed_error() {
		emit_error(
			"invalid number format - cannot declare a number with more that one '.' character"
		);
	}

	void compilation_logger::emit_invalid_number_format_unsigned_number_may_not_contain_dot_characters_error() {
		emit_error(
			"invalid number format - cannot declare an unsigned number containing '.' characters"
		);
	}

	void compilation_logger::emit_invalid_number_format_floating_point_must_contain_dot_character_error() {
		emit_error(
			"invalid number format - floating point number must contain a '.' character"
		);
	}

	void compilation_logger::emit_unterminated_character_literal_error() {
		emit_error(
			"invalid unterminated character literal detected"
		);
	}

	void compilation_logger::emit_unterminated_string_literal_error() {
		emit_error(
			"invalid unterminated string literal detected"
		);
	}

	void compilation_logger::emit_delete_file_failed_error(const std::string& filepath)	{
		emit_error(
			"cannot delete file '" + filepath + "'"
		);
	}

	void compilation_logger::emit_warning(u64 line_number, const std::string& message) {
		console::log(color::cyan, "[warning:" + std::to_string(line_number) + "]: ");
		console::log(color::white, message + '\n');
	}

	void compilation_logger::emit_error(u64 line_number, const std::string& message) {
		console::log(color::red, "[error:" + std::to_string(line_number) + "]: ");
		console::log(color::white, message + '\n');
	}

	void compilation_logger::emit_error(const std::string& message)	{
		console::log(color::red, "[error]: ");
		console::log(color::white, message + '\n');
	}
}