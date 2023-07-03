#include "string_accessor.h"

namespace sigma::detail {
	string_accessor::string_accessor(const std::string& string)
		: m_string(string) {}

	void string_accessor::advance()	{
		m_position++;
	}

	void string_accessor::retreat()	{
		m_position--;
	}

	char string_accessor::get() const {
		// check if we are inside of our strings' bounds
		ASSERT(m_position <= m_string.size(), "accessor out of range! (get)");
		return m_string[m_position];
	}

	char string_accessor::get_advance() {
		const char temp = get();
		advance();
		return temp;
	}

	bool string_accessor::end() const {
		return m_position > m_string.size();
	}

	u64 string_accessor::get_position() const {
		return m_position;
	}

	void string_accessor::set_position(u64 location) {
		// check if we are inside of our strings' bounds
		ASSERT(location <= m_string.size(), "accessor out of range! (set)");
		m_position = location;
	}
}