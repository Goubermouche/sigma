#pragma once
#include "intermediate_representation/codegen/targets/parameter_descriptor.h"
#include "intermediate_representation/codegen/architectures/x64/x64.h"

namespace ir::win {
	constexpr auto get_caller_saved() -> u16 {
		return
			1u << x64::rax |
			1u << x64::rcx | 
			1u << x64::rdx |
			1u << x64::r8  |
			1u << x64::r9  | 
			1u << x64::r10 |
			1u << x64::r11;
	}

	static const parameter_descriptor win_parameter_descriptor = {
		.gpr_count = 4,
		.xmm_count = 4,
		.caller_saved_xmm_count = 6,
		.caller_saved_gpr_count = get_caller_saved(),
		.gpr_registers = {
			x64::rcx, x64::rdx, x64::r8, x64::r9, reg::invalid_id, reg::invalid_id
		}
	};
}
