#include "memory.h"

namespace ir {
	reg::reg(id_type id) : id(id) {}

	auto reg::operator==(reg other) const -> bool {
		return id == other.id;
	}

	auto reg::is_valid() const -> bool {
		return id != invalid_id;
	}

	auto classified_reg::operator==(classified_reg other) const -> bool {
		return id == other.id && cl == other.cl;
	}
}
