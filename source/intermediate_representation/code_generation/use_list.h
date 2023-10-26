#pragma once
#include "intermediate_representation/code_generation/code_generator_context.h"

namespace ir::cg {
	/**
	 * \brief Generates usage lists for all nodes in the given \a context
	 * \param context Code generation context
	 */
	void generate_use_lists(code_generator_context& context);
}