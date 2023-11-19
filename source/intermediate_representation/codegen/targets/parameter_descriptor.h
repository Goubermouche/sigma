#pragma once
#include "intermediate_representation/codegen/memory/memory.h"

namespace ir {
	using namespace utility::types;

	struct parameter_descriptor {
		u16 gpr_count;
		u16 xmm_count;
		u16 caller_saved_xmm_count;
		u16 caller_saved_gpr_count;

		reg gpr_registers[6];
	};
}
