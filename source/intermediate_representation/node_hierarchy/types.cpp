#include "types.h"
#include <utility/macros.h>


namespace sigma::ir {
  auto data_type::to_string() const -> std::string {
    switch (ty) {
			case INTEGER:
        return "i" + std::to_string(bit_width);
			case POINTER:
        return "ptr" + std::to_string(bit_width);
			case FLOAT:
        return "f" + std::to_string(bit_width);
			case TUPLE:
        return "tuple";
			case CONTROL:
        return "control";
			case MEMORY:
        return "memory";
			default:
        NOT_IMPLEMENTED();
        return "";
    }
  }

  auto data_type::operator==(const data_type& other) const -> bool {
    return ty == other.ty && bit_width == other.bit_width;
  }
} // namespace sigma::ir
