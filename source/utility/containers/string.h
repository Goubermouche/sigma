#pragma once
#include "utility/types.h"
#include "utility/containers/iterators/iterator.h"
#include "utility/containers/iterators/const_iterator.h"

namespace utility {
	class string {
	public:
		string() = default;
		string(const char* value)
			: m_string(value) {}

		string(const std::string& value)
			: m_string(value) {}

		template<typename... arguments>
		string(const std::format_string<arguments...> fmt, arguments&&... args) {
			m_string.append(std::vformat(fmt.get(), std::make_format_args(args...)));
		}

		~string() = default;

		void operator+=(const string& other) {
			m_string += other.m_string;
		}

		template<typename... arguments>
		void append(
			const std::format_string<arguments...> fmt,
			arguments&&... args
		) {
			m_string.append(std::vformat(fmt.get(), std::make_format_args(args...)));
		}

		void append(const string& other) {
			m_string += other.get_underlying();
		}

		auto begin() -> std::string::iterator {
			return m_string.begin();
		}

		auto end() -> std::string::iterator {
			return m_string.end();
		}

		auto cbegin() const -> std::string::const_iterator {
			return m_string.cbegin();
		}

		auto cend() const -> std::string::const_iterator {
			return m_string.cend();
		}

		auto get_underlying() -> std::string {
			return m_string;
		}

		auto get_underlying() const -> const std::string& {
			return m_string;
		}
	private:
		std::string m_string;
	};
}

namespace std {
	inline ostream& operator<<(ostream& os, const utility::string& str) {
		os << str.get_underlying();
		return os;
	}
}
