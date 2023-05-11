#include "codegen_context.h"

namespace channel {
	codegen_context::codegen_context(type expected_type)
		: m_expected_type(expected_type) {}

	void codegen_context::set_expected_type(type expected_type) {
		m_expected_type = expected_type;
	}

	type codegen_context::get_expected_type() const	{
		return m_expected_type;
	}
}