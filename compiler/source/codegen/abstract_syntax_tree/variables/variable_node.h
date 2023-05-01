#pragma once
#include "../node.h"

namespace channel {
	/**
	 * \brief AST node, represents an access to an existing named value.
	 */
	class variable_node : public node {
	public:
		variable_node(
			u64 line_number,
			const std::string& variable_identifier
		);

		bool accept(visitor& visitor, value_ptr& out_value) override;
		void print(int depth, const std::wstring& prefix, bool is_last) override;

		const std::string& get_variable_identifier() const;
	private:
		std::string m_variable_identifier;
	};
}