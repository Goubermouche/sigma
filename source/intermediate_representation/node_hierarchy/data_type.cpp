#include "data_type.h"
#include <utility/macros.h>


namespace ir {
    auto data_type::to_string() const -> std::string {
        switch (ty) {
        case integer:
            return "i" + std::to_string(bit_width);
        case pointer:
            return "ptr" + std::to_string(bit_width);
        case floating_point:
            return "f" + std::to_string(bit_width);
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

    auto data_type::operator==(const data_type& other) const -> bool {
        return ty == other.ty && bit_width == other.bit_width;
    }
}
