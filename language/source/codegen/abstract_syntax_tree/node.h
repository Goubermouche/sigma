#pragma once
#include "../../utility/macros.h"
#include "../visitor.h"

namespace channel {
	class visitor;

	class node {
	public:
		virtual ~node() = default;
		virtual void accept(visitor& visitor) = 0;
	};
}