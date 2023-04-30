#pragma once
#include "../../node.h"

namespace channel {
	/**
	 * \brief AST node, assignment of an array value.
	 */
    class array_assignment_node : public node {
    public:
        array_assignment_node(u64 line_number, node* array_base, const std::vector<node*>& index_nodes, node* expression_node);
        bool accept(visitor& visitor, value*& out_value) override;
        void print(int depth, const std::wstring& prefix, bool is_last) override;

        node* get_array_base_node() const;
        const std::vector<node*>& get_array_element_index_nodes() const;
        node* get_expression_node() const;
    private:
        node* m_array_base_node;
        std::vector<node*> m_array_element_index_nodes;
        node* m_expression_node;
    };
}