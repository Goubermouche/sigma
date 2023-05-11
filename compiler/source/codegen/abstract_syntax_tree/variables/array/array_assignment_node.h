#pragma once
#include "codegen/abstract_syntax_tree/node.h"

namespace channel {
	/**
	 * \brief AST node, assignment to an array.
	 */
    class array_assignment_node : public node {
    public:
        array_assignment_node(
            const token_position& position,
            const node_ptr& array_base,
            const std::vector<node_ptr>& index_nodes,
            const node_ptr& expression_node
        );

        bool accept(visitor& visitor, value_ptr& out_value, codegen_context context) override;
        void print(int depth, const std::wstring& prefix, bool is_last) override;

        const node_ptr& get_array_base_node() const;
        const std::vector<node_ptr>& get_array_element_index_nodes() const;
        const node_ptr& get_expression_node() const;
    private:
        node_ptr m_array_base_node;
        std::vector<node_ptr> m_array_element_index_nodes;
        node_ptr m_expression_node;
    };
}