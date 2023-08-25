#pragma once
#include "intermediate_code/value.h"

namespace ir {
	class constant;
	using constant_ptr = ptr<constant>;

	/**
	 * \brief Base constant class type. Represents a specific value.
	 */
	class constant : public value {
	public:
		constant(const std::string& name);
	};
}
