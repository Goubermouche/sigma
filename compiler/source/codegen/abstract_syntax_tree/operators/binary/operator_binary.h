#pragma once
#include "../../node.h"

namespace channel {
    /**
     * \brief Base AST node, all binary operators derive from this node.
     */
	class operator_binary : public node	{
    public:
        operator_binary(u64 line_number, node* left_expression_node, node* right_expression_node);
        ~operator_binary() override;

        node* left_expression_node;
        node* right_expression_node;
	};
}