#include "value.h"

namespace ir {
	value::value(const std::string& name)
		: m_name(name) {}

	value::value(const value& other)
		: m_name(other.m_name) {}

	value::value(value&& other) noexcept
		: m_name(std::move(other.m_name)) {}

	value& value::operator=(const value& other) {
		if(this != & other) {
			m_name = other.m_name;
		}

		return *this;
	}

	value& value::operator=(value&& other) noexcept {
		if(this != &other) {
			m_name = std::move(other.m_name);
		}

		return *this;
	}

	const std::string& value::get_name() const {
		return m_name;
	}

	std::string value::to_string() {
		return m_name + " (default to_string impl)";
	}

	std::string value::get_value_string() {
		return "@" + m_name;
	}
}
