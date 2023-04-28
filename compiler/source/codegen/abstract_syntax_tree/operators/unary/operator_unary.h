#pragma once
#include "../../node.h"

namespace channel {
    /**
     * \brief Base AST node, all binary operators derive from this node.
     */
    class operator_unary : public node {
    public:
        operator_unary(u64 line_number, node* expression_node);
        ~operator_unary() override;

    private:
        node* m_expression_node;
    };
}