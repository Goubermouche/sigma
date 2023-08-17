#include "warning.h"
#include "string_helper.h"

namespace sigma {
	warning_message::warning_message(
		warning_code code,
		const std::string& message
	) : diagnostic_message(message),
		m_code(code) {}

	void warning_message::print() {
		console::out
			<< color::yellow
			<< "warning:" << static_cast<u64>(m_code)
			<< color::white << ": " << m_message << '\n';
	}

	warning_message_range::warning_message_range(
		warning_code code, 
		const std::string& message,
		const file_range& range
	) : warning_message(code, message),
	m_range(range) {}

	void warning_message_range::print()	{
		ASSERT(m_range.file != nullptr, "file was nullptr!");

		console::out
			<< color::white
			<< m_range.file->get_path().string()
			<< ":" << m_range.start.line_index + 1
			<< ":" << m_range.start.char_index
			<< color::yellow << ":warning:" << static_cast<u64>(m_code)
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
			<< color::yellow << "    "
			<< detail::create_caret_underline(
				line,
				m_range.start.char_index - removed_char_count,
				m_range.end.char_index - removed_char_count)
			<< color::white << '\n';
	}
}