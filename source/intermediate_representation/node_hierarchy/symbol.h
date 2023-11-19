#pragma once
#include <utility/types.h>

namespace ir {
	struct symbol {
		enum class tag {
			none,

			// symbol is dead
			tombstone,

			external,
			global,
			function,

			max
		};

		std::string name;
		tag t;
	};
}