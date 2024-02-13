#pragma once
#include "types.h"

namespace utility {
	template<typename type>
	struct range {
		static auto max() -> range {
			return {
				std::numeric_limits<type>::max(),
				std::numeric_limits<type>::max()
			};
		}

		static auto min() -> range {
			return {
				std::numeric_limits<type>::min(),
				std::numeric_limits<type>::min()
			};
		}

		type start;
		type end;
	};

	template<typename type>
	auto range_intersect(range<type>& a, range<type>& b) -> ptr_diff {
		if (b.start <= a.end && a.start <= b.end) {
			return static_cast<ptr_diff>(a.start > b.start ? a.start : b.start);
		}

		return -1;
	}
}
