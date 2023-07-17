#include "warning.h"

namespace sigma {
	warning_message::warning_message(
		std::string message,
		u64 code
	) : diagnostic_message(std::move(message), code) {}

	void warning_message::print() const {
		console::out
			<< color::blue
			<< "warning "
			<< m_code
			<< ": "
			<< m_message
			<< "\n"
			<< color::white;
	}

	warning_message_position::warning_message_position(
		std::string message,
		u64 code,
		const file_position& position
	) : warning_message(std::move(message), code),
	m_position(position) {}

	void warning_message_position::print() const {
		console::out
			<< m_position.get_path().string() 
			<< " ("
			<< m_position.get_line_index()
			<< ", "
			<< m_position.get_char_index()
			<< "): "
			<< color::yellow
			<< "warning "
			<< m_code
			<< ": "
			<< m_message
			<< "\n"
			<< color::white;
	}
}