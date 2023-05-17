#include "code_generation_context.h"

namespace channel {
	code_generation_context::code_generation_context(type expected_type)
		: m_expected_type(expected_type) {}

	void code_generation_context::set_expected_type(type expected_type) {
		m_expected_type = expected_type;
	}

	type code_generation_context::get_expected_type() const	{
		return m_expected_type;
	}
}
