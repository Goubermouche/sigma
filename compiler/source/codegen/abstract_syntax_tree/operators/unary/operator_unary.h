#pragma once
#include "../../node.h"

namespace channel {
    /**
     * \brief Base AST node, all unary operators derive from this node.
     */
    class operator_unary : public node {
    public:
        operator_unary(
            u64 line_number,
            const node_ptr& expression_node
        );

        const node_ptr& get_expression_node() const;
    private:
        node_ptr m_expression_node;
    };
}