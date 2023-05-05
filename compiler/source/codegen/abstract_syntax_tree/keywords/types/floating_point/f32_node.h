#pragma once
#include "../../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents an f32 literal.
	 */
	class f32_node : public node {
	public:
		f32_node(
			const token_position& position, 
			f32 value
		);

		bool accept(visitor& visitor, value_ptr& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;

		f32 get_value() const;
	private:
		f32 m_value;
	};
}