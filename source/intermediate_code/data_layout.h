#pragma once
#include <utility/macros.h>

using namespace utility::types;

namespace ir {
	/**
	 * \brief Basic alignment wrapper, used in various instructions. 
	 */
	struct alignment {
		alignment(u64 value);

		u64 get_value() const;
	private:
		u64 m_value; // alignment, in bytes
	};

	/**
	 * \brief Represents data layout on a given system.
	 */
	class data_layout {
	public:
		data_layout(u64 preferred_alignment);

		/**
		 * \brief Gets the preferred byte alignment for the given system
		 * \param bit_width Bit width to align
		 * \return Byte aligned bit width.
		 */
		u64 get_alignment_for_bit_width(u64 bit_width) const;
	private:
		u64 m_preferred_alignment;
	};
}