#pragma once
#include "../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents a function.
	 */
	class return_node : public node {
	public:
		return_node(u64 line_number, node* return_expression_node);
		bool accept(visitor& visitor, value*& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;

		node* get_return_expression_node() const;
	private:
		node* m_return_expression_node;
	};
}