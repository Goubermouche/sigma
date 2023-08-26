#pragma once
#include "intermediate_code/types/type.h"

namespace ir {
	class constant;
	using constant_ptr = ptr<constant>;

	/**
	 * \brief Base constant class type. Represents a specific value.
	 */
	class constant : public value {
	public:
		constant(type_ptr type, const std::string& name);

		type_ptr get_type() const;
	protected:
		type_ptr m_type;
	};
}
