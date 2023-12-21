#include "types.h"
#include <utility/macros.h>

namespace sigma::ir {
	data_type::data_type() : m_base(base::UNKNOWN), m_bit_width(0) {}
  data_type::data_type(base base, u8 bit_width) : m_base(base), m_bit_width(bit_width) {}

  auto data_type::to_string() const -> std::string {
    switch (m_base) {
			case base::INTEGER:
        return "i" + std::to_string(m_bit_width);
			case base::POINTER:
        return "ptr" + std::to_string(m_bit_width);
			case base::FLOAT:
        return "f" + std::to_string(m_bit_width);
			case base::TUPLE:
        return "tuple";
			case base::CONTROL:
        return "control";
			case base::MEMORY:
        return "memory";
	    case base::UNKNOWN:
				return "unknown";
			default:
        NOT_IMPLEMENTED();
        return "";
    }
  }

  auto data_type::operator==(const data_type& other) const -> bool {
    return m_base.get_underlying() == other.m_base.get_underlying() && m_bit_width == other.m_bit_width;
  }

	auto data_type::get_base() const -> base {
		return m_base;
	}

	auto data_type::get_bit_width() const -> u8 { 
		return m_bit_width;
	}

	void data_type::set_bit_width(u8 bit_width) {
		m_bit_width = bit_width;
	}

	data_type::base::base() : m_type(UNKNOWN) {}

	data_type::base::base(underlying type) : m_type(type) {}

	data_type::base::operator underlying() const{
		return m_type;
	}

	auto data_type::base::get_underlying() const -> underlying {
		return m_type;
	}

	bool operator==(data_type type_a, data_type::base::underlying type_b) {
		return type_a.get_base().get_underlying() == type_b;
	}

} // namespace sigma::ir
