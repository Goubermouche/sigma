#include "data_type.h"
#include <utility/macros.h>

namespace ir {
    data_type::data_type(id type, u8 bit_width)
	    : m_id(type), m_bit_width(bit_width) {}

    data_type::data_type(id type)
	    : m_id(type), m_bit_width(0) {}

    std::string data_type::to_string() {
        switch (m_id) {
        case integer:
            return "i" + std::to_string(m_bit_width);
        case pointer:
            return "ptr" + std::to_string(m_bit_width);
        case floating_point:
            return "f" + std::to_string(m_bit_width);
        case tuple:
            return "tuple";
        case control:
            return "control";
        case memory:
            return "memory";
        default:
            ASSERT(false, "not implemented");
            return "";
        }
    }

    void data_type::set_bit_width(u8 bit_width) {
        m_bit_width = bit_width;
    }

    u8 data_type::get_bit_width() const {
        return m_bit_width;
    }

    bool data_type::operator==(const data_type& other) const {
        return m_id == other.m_id && m_bit_width == other.m_bit_width;
    }

    void data_type::set_id(id id) {
        m_id = id;
    }

    data_type::id data_type::get_id() const {
        return m_id;
    }
}
