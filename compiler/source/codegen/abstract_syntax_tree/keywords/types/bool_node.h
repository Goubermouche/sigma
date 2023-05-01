#pragma once
#include "../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents a boolean literal.
	 */
	class bool_node : public node {
	public:
		bool_node(u64 line_number, bool value);

		bool accept(visitor& visitor, value_ptr& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;

		bool get_value() const;
	private:
		bool m_value;
	};
}