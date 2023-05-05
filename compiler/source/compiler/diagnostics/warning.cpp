#include "warning.h"

namespace channel {
	warning_message::warning_message(std::string message, u64 code)
		: diagnostic_message(message, code) {}

	void warning_message::print() const {
		console::out
			<< color::blue
			<< "warning"
			<< " ("
			<< m_code
			<< "): "
			<< color::white
			<< m_message
			<< '\n';
	}

	warning_message_position::warning_message_position(std::string message, u64 code, token_position position)
		: warning_message(std::move(message), code), m_position(position) {}

	void warning_message_position::print() const {
		console::out
			<< "("
			<< m_position.get_line_number()
			<< ", "
			<< m_position.get_character_number()
			<< "): "
			<< color::orange
			<< "warning "
			<< m_code
			<< ": "
			<< m_message
			<< "\n"
			<< color::white;
	}
}