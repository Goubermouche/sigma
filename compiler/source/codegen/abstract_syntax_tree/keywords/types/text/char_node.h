#pragma once
#include "../../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents the i64 type keyword.
	 */
	class char_node : public node {
	public:
		char_node(u64 line_number, char value);
		bool accept(visitor& visitor, value*& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;

		char get_value() const;
	private:
		char m_value;
	};
}
