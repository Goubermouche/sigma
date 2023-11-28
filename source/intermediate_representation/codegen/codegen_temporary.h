#pragma once
#include "intermediate_representation/codegen/instruction.h"

namespace ir {
	struct codegen_context;

	struct codegen_temporary : utility::property<absolute, label, handle<symbol>> {
		enum value_type {
			none,
			flags,
			gpr,
			xmm,
			imm,
			mem,
			global,
			abs,
			label
		};

		static auto create_label(codegen_context& context, u64 target) -> handle<codegen_temporary>;
		static auto create_imm(codegen_context& context,i32 imm) -> handle<codegen_temporary>;
		static auto create_abs(codegen_context& context,u64 abs) -> handle<codegen_temporary>;

		auto matches(handle<codegen_temporary> b) const -> bool;

		value_type type;
		u8 reg;
		u8 index;
		scale sc;
		i32 immediate;
	};
}
