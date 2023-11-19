#pragma once
#include "intermediate_representation/codegen/instruction.h"

namespace ir {
	struct codegen_context;

	struct value : utility::property<absolute, label, handle<symbol>> {
		enum type {
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

		static auto create_label(codegen_context& context, u64 target) -> handle<value>;
		static auto create_imm(codegen_context& context,i32 imm) -> handle<value>;
		static auto create_abs(codegen_context& context,u64 abs) -> handle<value>;

		auto matches(handle<value> b) const -> bool;

		type m_type;
		u8 m_reg;
		u8 m_index;
		scale m_scale;
		i32 m_imm;
	};
}
