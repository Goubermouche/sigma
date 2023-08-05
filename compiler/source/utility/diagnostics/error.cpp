#include "error.h"
#include "utility/string.h"

namespace sigma {
	error_message::error_message(
		error_code code,
		const std::string& message
	) : diagnostic_message(message),
	m_code(code) {}

	void error_message::print() {
		console::out
			<< color::red
			<< "error:" << static_cast<u64>(m_code)
			<< color::white << ": " << m_message << '\n';
	}

	error_message_range::error_message_range(
		error_code code, 
		const std::string& message, 
		const file_range& range
	) : error_message(code, message),
	m_range(range) {}

	void error_message_range::print() {
		ASSERT(m_range.file != nullptr, "file was nullptr!");

		console::out
			<< color::white
			<< m_range.file->get_path().string()
			<< ":" << m_range.start.line_index + 1
			<< ":" << m_range.start.char_index
			<< color::red << ":error:" << static_cast<u64>(m_code)
			<< color::white << ": " << m_message << '\n';

		const auto& [removed_char_count, line] = detail::remove_leading_spaces(
			m_range.file->get_line(
				m_range.start.line_index
			)
		);

		console::out
			<< color::white << "    "
			<< line << '\n';

		console::out
			<< color::red << "    "
			<< detail::create_caret_underline(
				line,
				m_range.start.char_index - removed_char_count,
				m_range.end.char_index - removed_char_count)
			<< color::white << '\n';
	}
}