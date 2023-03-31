#pragma once
#include "abstract_syntax_tree/node.h"

namespace channel {
	// node predefinitions 
	// keywords
	class keyword_i32_node;

	// operators
	class operator_addition_node;

	/**
	 * \brief Codegen visitor that walks the AST and generates LLVM IR.
	 */
	class visitor {
	public:
		virtual ~visitor() = default;

		// keywords
		virtual void visit_keyword_i32_node(keyword_i32_node& node) = 0;

		// operators
		virtual void visit_operator_addition_node(operator_addition_node& node) = 0;
	};
}