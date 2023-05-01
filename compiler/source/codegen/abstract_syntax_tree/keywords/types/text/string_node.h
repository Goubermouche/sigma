#pragma once
#include "../../../node.h"

namespace channel {
	/**
	 * \brief AST node, represents a string literal.
	 */
	class string_node : public node {
	public:
		string_node(u64 line_number, const std::string& value);

		bool accept(visitor& visitor, value_ptr& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;

		const std::string& get_value() const;
	private:
		std::string m_value;
	};
}
