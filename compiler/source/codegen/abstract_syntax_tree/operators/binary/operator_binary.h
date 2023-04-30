#pragma once
#include "../../node.h"

namespace channel {
    /**
     * \brief Base AST node, all binary operators derive from this node.
     */
	class operator_binary : public node	{
    public:
        operator_binary(u64 line_number, node_ptr left_expression_node, node_ptr right_expression_node);
        ~operator_binary() override;

        node_ptr get_left_expression_node() const;
        node_ptr get_right_expression_node() const;
	private:
        node_ptr m_left_expression_node;
        node_ptr m_right_expression_node;
	};
}