#include "integer_type.h"

namespace ir {
	integer_type::integer_type(
		u16 bit_width,
		bool is_signed
	) : type(""),
	m_bit_width(bit_width),
	m_is_signed(is_signed) {}

	integer_type_ptr integer_type::create(u16 bit_width, bool is_signed) {
		return std::make_shared<integer_type>(bit_width, is_signed);
	}

	std::string integer_type::to_string() {
		return (m_is_signed ? "i" : "u") + std::to_string(m_bit_width);
	}
}