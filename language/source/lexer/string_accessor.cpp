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

	bool string_accessor::get(char& out) {
		if (m_position < m_string.size()) {
			out = m_string[m_position];
			return true;
		}

		return false;
	}

	bool string_accessor::end() {
		return m_position > m_string.size();
	}
}