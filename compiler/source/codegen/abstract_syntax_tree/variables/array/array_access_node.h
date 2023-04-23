#pragma once
#include "../../node.h"

namespace channel {
    class array_access_node : public node {
    public:
        array_access_node(u64 line_number, node* array_base, const std::vector<node*>& array_element_index_nodes);
        bool accept(visitor& visitor, value*& out_value) override;
        std::string get_node_name() const override;

        node* get_array_base_node() const;
        const std::vector<node*>& get_array_element_index_nodes() const;
    private:
        node* m_array_base;
        std::vector<node*> m_array_element_index_nodes;
    };
}