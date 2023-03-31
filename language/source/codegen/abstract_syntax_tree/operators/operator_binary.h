#pragma once
#include "../node.h"

namespace channel {
    /**
     * \brief Base AST node, all binary operators derive from this node.
     */
	class operator_binary : public node	{
    public:
        operator_binary(node* left, node* right);
        ~operator_binary();

        node* left;
        node* right;
	};
}