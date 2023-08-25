#pragma once
#include "intermediate_code/value.h"

namespace ir {
	class type;
	using type_ptr = ptr<type>;

	/**
	 * \brief Base type class type. Contains information about a given type.
	 */
	class type : public value {
	public:
		type(const std::string& name);
	};
}
