#pragma once
#include "../node.h"

namespace channel {
	/**
	 * \brief AST node, represents a variable.
	 */
	class access_node : public node {
	public:
		access_node(u64 line_number, const std::string& variable_identifier);
		bool accept(visitor& visitor, value*& out_value) override;
		std::string get_node_name() const override;

		const std::string& get_variable_identifier() const;
	private:
		std::string m_variable_identifier;
	};
}