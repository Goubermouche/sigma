#include "diagnostic.h"

namespace channel {
	diagnostic_message::diagnostic_message(std::string message, u64 code)
		: m_message(message), m_code(code) {}
}
