#include "compilation_logger.h"

namespace channel {
	void compilation_logger::emit_cast_warning(u64 line_number, type original_type, type target_type) {
		emit_warning(
			line_number,
			"implicit type cast ('" + type_to_string(original_type) + "' to '" + type_to_string(target_type) + "')"
		);
	}

	void compilation_logger::emit_function_return_type_cast_warning(u64 line_number, type original_type, type target_type) {
		emit_warning(
			line_number,
			"implicit function return type cast ('" + type_to_string(original_type) + "' to '" + type_to_string(target_type) + "')"
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

	void compilation_logger::emit_unhandled_number_format_error(u64 line_number, token received_token) {
		emit_error(
			line_number,
			"unhandled numerical token received ('" + token_to_string(received_token) + "')"
		);
	}

	void compilation_logger::emit_warning(u64 line_number, const std::string& message) {
		console::log(
			console::color(color::cyan, "[warning:" + std::to_string(line_number) + "]: ") + message
		);
	}

	void compilation_logger::emit_error(u64 line_number, const std::string& message) {
		console::log(
			console::color(color::red, "[error:" + std::to_string(line_number) + "]: ")	+ message
		);
	}
}