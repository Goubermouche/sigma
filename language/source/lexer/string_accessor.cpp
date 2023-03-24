#include "string_accessor.h"

namespace language::detail {
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
		if (m_position <= m_string.size()) {
			return m_string[m_position];
		}

		ASSERT(false, "accessor out of range!");
		return ' ';
	}

	char string_accessor::get_advance() {
		const char temp = get();
		advance();
		return temp;
	}

	bool string_accessor::end() const {
		return m_position > m_string.size();
	}
}