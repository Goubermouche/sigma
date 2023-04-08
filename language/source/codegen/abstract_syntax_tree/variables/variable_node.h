#pragma once
#include "../node.h"

namespace channel {
	/**
	 * \brief AST node, represents a variable.
	 */
	class variable_node : public node {
	public:
		variable_node(u64 line_index, const std::string& name);
		value* accept(visitor& visitor) override;
		std::string get_node_name() const override;

		const std::string& get_name() const;
	private:
		std::string m_name;
	};
}
