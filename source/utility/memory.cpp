#include "memory.h"

namespace utility {
    byte::byte(u8 value)
	    : value(value) {}

    std::string byte::get_hex() const {
        std::ostringstream oss;
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(value);
        return oss.str();
    }
}