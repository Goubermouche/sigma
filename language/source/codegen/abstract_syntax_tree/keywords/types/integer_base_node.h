#pragma once
#include "../../node.h"

namespace channel {
	class integer_base_node : public node {
	public:
		virtual bool is_signed() const = 0;
	};
}

