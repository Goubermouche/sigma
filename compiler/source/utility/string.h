#pragma once
#include "utility/macros.h"

namespace sigma::detail {
	std::string format_ending(
		u64 count,
		const std::string& singular,
		const std::string& plural
	);
}