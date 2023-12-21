#pragma once
#include "intermediate_representation/codegen/codegen_context.h"

namespace sigma::ir {
	void determine_live_ranges(codegen_context& context);
} // namespace sigma::ir
