#pragma once
#include <util/types.h>

namespace sigma::ir {
	using namespace utility::types;

	struct reg {
		using id_type = u8;

		reg() = default;
		reg(id_type id);
		
		auto operator==(reg other) const -> bool;
		auto is_valid() const -> bool;

		static constexpr id_type invalid_id = std::numeric_limits<id_type>::max();
		id_type id = invalid_id;
	};

	struct classified_reg : reg {
		using class_type = u8;

		auto operator==(classified_reg other) const -> bool;

		static constexpr class_type invalid_class = std::numeric_limits<class_type>::max();
		class_type cl = invalid_class;
	};

	enum class memory_scale : u8 {
		x1, x2, x4, x8
	};

	struct mem {
		u8 index;
		memory_scale scale;
		i32 displacement;
	};
} // namespace sigma::ir
