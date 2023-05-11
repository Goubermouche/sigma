#pragma once
#include "codegen/abstract_syntax_tree/node.h"

namespace channel {
    /**
     * \brief AST node, represents an access to an array.
     */
    class array_access_node : public node {
    public:
        array_access_node(
            const token_position& position,
            const node_ptr& array_base,
            const std::vector<node_ptr>& array_element_index_nodes
        );

        acceptation_result accept(
            visitor& visitor,
            const codegen_context& context
        ) override;

        void print(u64 depth, const std::wstring& prefix, bool is_last) override;

        const node_ptr& get_array_base_node() const;
        const std::vector<node_ptr>& get_array_element_index_nodes() const;
    private:
        node_ptr m_array_base;
        std::vector<node_ptr> m_array_element_index_nodes;
    };
}