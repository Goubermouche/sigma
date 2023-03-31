#pragma once
#include "visitor.h"

namespace channel {
	/**
	 * \brief Evaluator that implements the codegen visitor to generate LLVM IR.
	 */
	class evaluator : public visitor {
		// keywords
		void visit_keyword_i32_node(keyword_i32_node& node) override;

		// operators
		void visit_operator_addition_node(operator_addition_node& node) override;
	};
}