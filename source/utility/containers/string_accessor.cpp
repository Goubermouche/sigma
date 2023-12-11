#include "string_accessor.h"
#include "utility/macros.h"

namespace utility::detail {
	string_accessor::string_accessor(const std::string& string)
		: m_string(string) {}

	void string_accessor::advance()	{
		m_position++;
	}

	void string_accessor::retreat()	{
		m_position--;
	}

	auto string_accessor::get() const -> char {
		// check if we are inside of our strings' bounds
		ASSERT(m_position <= m_string.size(), "accessor out of range! (get)");
		return m_string[m_position];
	}

	auto string_accessor::get_advance() -> char {
		const char temp = get();
		advance();
		return temp;
	}

	auto string_accessor::end() const -> bool {
		return m_position > m_string.size();
	}

	auto string_accessor::get_data() const -> const std::string& {
		return m_string;
	}

	auto string_accessor::get_data() -> std::string& {
		return m_string;
	}

	auto string_accessor::get_position() const -> u64 {
		return m_position;
	}

	void string_accessor::set_position(u64 position) {
		// check if we are inside of our strings' bounds
		ASSERT(position <= m_string.size(), "accessor out of range! (set)");
		m_position = position;
	}
}
