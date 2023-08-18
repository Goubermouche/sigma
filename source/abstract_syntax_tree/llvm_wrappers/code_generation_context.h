#pragma once
#include "abstract_syntax_tree/llvm_wrappers/type.h"

namespace sigma {
	/**
	 * \brief Code generation context, holds variables that may be used in the next accept call.
	 */
	struct code_generation_context {
		code_generation_context() = default;
		code_generation_context(
			type expected_type
		);

		void set_expected_type(type expected_type);
		type get_expected_type() const;
	private:
		type m_expected_type;
	};
}