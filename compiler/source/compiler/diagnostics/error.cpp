#include "error.h"

namespace channel {
	error_message::error_message(std::string message, u64 code)
		: diagnostic_message(message, code) {}

	void error_message::print() const {
		console::out
			<< color::red
			<< "error "
			<< m_code
			<< ": "
			<< m_message
			<< "\n"
			<< color::white;
	}

	error_message_position::error_message_position(std::string message, u64 code, token_position position)
		: error_message(std::move(message), code), m_position(position) {}

	void error_message_position::print() const {
		console::out
			<< m_position.get_file()
			<< " ("
			<< m_position.get_line_number()
			<< ", "
			<< m_position.get_character_number()
			<< "): "
			<< color::red
			<< "error "
			<< m_code
			<< ": "
			<< m_message
			<< "\n"
			<< color::white;
	}
}
