#include "string_accessor.h"

namespace utility::detail {
	string_accessor::string_accessor(const std::string& string)
		: m_string(string) {}

	string_accessor::string_accessor(ptr<text_file> file) {
		// compute total length of the result string
		const u64 total_length = std::accumulate(file->get_lines().begin(), file->get_lines().end(), size_t{},
			[](u64 sum, const std::string& str) {
				return sum + str.size() + 1;  // +1 for newline character
			});

		m_string.reserve(total_length);

		for (const auto& str : file->get_lines()) {
			std::ranges::copy(str.begin(), str.end(), std::back_inserter(m_string));
			m_string.push_back('\n');
		}
	}

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

	void string_accessor::set_position(u64 position) {
		// check if we are inside of our strings' bounds
		ASSERT(position <= m_string.size(), "accessor out of range! (set)");
		m_position = position;
	}
}
