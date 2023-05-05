#pragma once
#include "../../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents a char literal.
	 */
	class char_node : public node {
	public:
		char_node(
			const token_position& position, 
			char value
		);

		bool accept(visitor& visitor, value_ptr& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;

		char get_value() const;
	private:
		char m_value;
	};
}
