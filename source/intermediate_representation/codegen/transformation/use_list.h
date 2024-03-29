#pragma once
#include "intermediate_representation/codegen/transformation/transformation_context.h"

namespace sigma::ir {
	/**
	 * \brief Generates usage lists for all nodes in the given \a context
	 * \param context Code generation context
	 */
	void generate_use_lists(transformation_context& context);
} // namespace sigma::ir
