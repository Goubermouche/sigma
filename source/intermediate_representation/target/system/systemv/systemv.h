#pragma once
#include "intermediate_representation/target/parameter_descriptor.h"
#include "intermediate_representation/target/arch/x64/x64.h"

namespace sigma::ir::systemv {
	constexpr auto get_caller_saved() -> u16 {
		return
			1u << x64::gpr::RAX |
			1u << x64::gpr::RDI |
			1u << x64::gpr::RSI |
			1u << x64::gpr::RCX |
			1u << x64::gpr::RDX |
			1u << x64::gpr::R8  |
			1u << x64::gpr::R9  |
			1u << x64::gpr::R10 |
			1u << x64::gpr::R11;
	}

	static const parameter_descriptor parameter_descriptor = {
		.gpr_count = 6,
		.xmm_count = 4,
		.caller_saved_xmm_count = 5,
		.caller_saved_gpr_count = get_caller_saved(),
		.gpr_registers = {
			static_cast<u8>(x64::gpr::RDI),
			static_cast<u8>(x64::gpr::RSI),
			static_cast<u8>(x64::gpr::RDX),
			static_cast<u8>(x64::gpr::RCX),
			static_cast<u8>(x64::gpr::R8),
			static_cast<u8>(x64::gpr::R9)
		}
	};
}
