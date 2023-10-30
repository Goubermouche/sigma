#include "warning.h"
#include "utility/string_helper.h"

namespace utility {
	warning_message::warning_message(
		warning_code code,
		const std::string& message
	) : diagnostic_message(message),
		m_bytes(code) {}

	console& operator<<(
		console& console,
		const warning_message& m
		) {
		console::out
			<< color::yellow
			<< "warning:" << static_cast<u64>(m.m_bytes)
			<< color::white << ": " << m.m_message << '\n';

		return console;
	}

	warning_message_range::warning_message_range(
		warning_code code, 
		const std::string& message,
		const file_range& range
	) : warning_message(code, message),
	m_range(range) {}

	console& operator<<(
		console& console, 
		const warning_message_range& m
	) {
		ASSERT(m.m_range.file != nullptr, "file was nullptr!");

		console::out
			<< color::white
			<< m.m_range.file->get_path().string()
			<< ":" << m.m_range.start.line_index + 1
			<< ":" << m.m_range.start.char_index
			<< color::yellow << ":warning:" << static_cast<u64>(m.m_bytes)
			<< color::white << ": " << m.m_message << '\n';

		const auto& [removed_char_count, line] = detail::remove_leading_spaces(
			m.m_range.file->get_line(
				m.m_range.start.line_index
			)
		);

		console::out
			<< color::white << "    "
			<< line << '\n';

		console::out
			<< color::yellow << "    "
			<< detail::create_caret_underline(
				line,
				m.m_range.start.char_index - removed_char_count,
				m.m_range.end.char_index - removed_char_count)
			<< color::white << '\n';

		return console;
	}
}