#include "data_layout.h"

namespace ir {
	alignment::alignment(u64 value)
		: m_value(value) {}

	u64 alignment::get_value() const {
		return m_value;
	}

	data_layout::data_layout(u64 preferred_alignment)
		: m_preferred_alignment(preferred_alignment) {}

	u64 data_layout::get_alignment_for_bit_width(u64 bit_width) const {
		if (bit_width == 0) {
			return 1; // default to 1 byte alignment
		}

		const u64 byte_width = (bit_width + 7) / 8;
		u64 alignment = 1;

		// determine alignment as the next power of two, capped at m_preferred_alignment.
		while (alignment < byte_width && alignment < m_preferred_alignment) {
			alignment *= 2;
		}

		return alignment;
	}
}