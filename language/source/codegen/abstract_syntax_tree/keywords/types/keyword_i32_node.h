#pragma once
#include "../../node.h"

namespace channel {
	class keyword_i32_node : public node {
		void accept(visitor& visitor) override;
		i32 value;
	};
}
