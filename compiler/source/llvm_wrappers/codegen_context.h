#pragma once
#include "llvm_wrappers/type.h"

namespace channel {
	struct codegen_context {
		codegen_context() = default;
		codegen_context(
			type expected_type
		);

		void set_expected_type(type expected_type);
		type get_expected_type() const;
	private:
		type m_expected_type;
	};
}