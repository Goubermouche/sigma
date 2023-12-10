#pragma once
#include "intermediate_representation/target/parameter_descriptor.h"
#include "intermediate_representation/target/arch/x64/x64.h"

namespace sigma::ir::systemv {
	constexpr auto get_caller_saved() -> u16 {
		return
			1u << x64::RAX |
			1u << x64::RDI |
			1u << x64::RSI |
			1u << x64::RCX |
			1u << x64::RDX |
			1u << x64::R8  |
			1u << x64::R9  |
			1u << x64::R10 |
			1u << x64::R11;
	}

	static const parameter_descriptor parameter_descriptor = {
		.gpr_count = 6,
		.xmm_count = 4,
		.caller_saved_xmm_count = 5,
		.caller_saved_gpr_count = get_caller_saved(),
		.gpr_registers = {
			x64::RDI, x64::RSI, x64::RDX, x64::RCX, x64::R8, x64::R9
		}
	};
}
