#pragma once
#include "../utility/macros.h"

namespace language::detail {
	class string_accessor {
	public:
		string_accessor() = default;
		string_accessor(const std::string& string);

		void advance();
		void retreat();
		bool get(char& out);
		bool end();
	private:
		std::string m_string;
		u64 m_position = 0;
	};
}