#pragma once
#include "utility/diagnostics/console.h"

namespace utility {
	struct byte {
		byte(u8 value);
		std::string get_hex() const;

		u8 value;
	};
}