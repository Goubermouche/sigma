#pragma once
#include "intermediate_representation/code_generation/code_generator_context.h"

namespace ir::cg {
	void determine_live_ranges(
		code_generator_context& context
	);
}

