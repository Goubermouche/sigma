#pragma once
#include "intermediate_code/types/type.h"

namespace ir {
	class integer_type;
	using integer_type_ptr = ptr<integer_type>;

	/**
	 * \brief Integer type class. Contains information about an integer type.
	 */
	class integer_type : public type {
	public:
		integer_type(u16 bit_width, bool is_signed);

		/**
		 * \brief Creates a new integer type ptr instance. 
		 * \param bit_width Specific bit width of the integer
		 * \param is_signed Whether the integer is signed or unsigned 
		 * \return Integer type ptr
		 */
		static integer_type_ptr create(u16 bit_width, bool is_signed);

		std::string to_string() override;
	private:
		u16 m_bit_width;
		bool m_is_signed;
	};
}
