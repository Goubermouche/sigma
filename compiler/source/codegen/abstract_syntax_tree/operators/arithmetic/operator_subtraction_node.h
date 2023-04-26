#pragma once
#include "../operator_binary.h"

namespace channel {
	/**
	 * \brief AST node, represents the binary subtraction operator.
	 */
	class operator_subtraction_node : public operator_binary {
	public:
		operator_subtraction_node(u64 line_number, node* left, node* right);
		bool accept(visitor& visitor, value*& out_value) override;
		std::string get_node_name() const override;
	};
}
