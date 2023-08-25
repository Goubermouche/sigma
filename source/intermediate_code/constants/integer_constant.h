#pragma once
#include "intermediate_code/constants/constant.h"
#include "intermediate_code/types/integer_type.h"

namespace ir {
	class integer_constant;
	using integer_constant_ptr = ptr<integer_constant>;

	/**
	 * \brief Integer constant, represents a specific integer value.
	 */
	class integer_constant : public constant {
	public:
		integer_constant(integer_type_ptr type, u64 value);

		/**
		 * \brief Creates a new integer constant ptr instance.
		 * \param type Integral type of the constant
		 * \param value Actual value of the constant
		 * \return Integer constant ptr.
		 */
		static integer_constant_ptr create(integer_type_ptr type, u64 value);

		std::string to_string() override;
		std::string get_value_string() override;
	private:
		integer_type_ptr m_type;
		u64 m_value;
	};
}
