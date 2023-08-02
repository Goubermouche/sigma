#pragma once
#include "utility/macros.h"

namespace sigma::detail {
	template<typename key_type, typename value_type>
	class directed_graph_node {
    public:
        directed_graph_node() = default;
        directed_graph_node(
            const value_type& value,
            const std::vector<key_type>& children
        ) : m_value(value),
            m_children(children) {}

        const std::vector<key_type>& get_children() const {
            return m_children;
        }

        value_type& get_value() {
            return m_value;
        }
    private:
        value_type m_value;
        std::vector<key_type> m_children;
	};

    template<typename key_type, typename value_type>
	class directed_graph {
        static_assert(
            std::is_default_constructible_v<value_type>,
            "value_type must be default-constructible"
        );

        using node_type = directed_graph_node<key_type, value_type>;
        using function_type = std::function<void(const key_type&, node_type*)>;
    public:
        void add_node(
            const key_type& key,
            const value_type& value,
            const std::vector<key_type>& children = {}
        ) {
            m_nodes[key] = new node_type(value, children);
        }

        void print() const {
            for (const auto& [key, node] : m_nodes) {
                console::out << key << '\n';

                for (const auto& dependency : node->get_children()) {
                    console::out << "    " << dependency << '\n';
                }
            }

            console::out << '\n';
        }

        bool is_acyclic() const {
            std::unordered_map<key_type, bool> visited;
            std::unordered_map<key_type, bool> rec_stack;

            for (auto& node : m_nodes) {
                if (dfs_visit(node.first, visited, rec_stack)) {
                    return false;
                }
            }

            return true;
        }

        bool contains(
            const key_type& key
        ) const {
            return m_nodes.contains(key);
        }

        u64 size() const {
            return m_nodes.size();
        }

        node_type* get_node(const key_type& key) {
            return m_nodes[key];
        }

        value_type& get_value(const key_type& key) {
            return m_nodes[key]->get_value();
        }

        void post_order_traverse(
            const key_type& root,
            const function_type& function,
            bool unique
        ) {
            if(unique) {
                std::unordered_set<key_type> visited;
                post_order_traverse_unique(root, function, visited);
            }
            else {
                post_order_traverse(root, function);
            }
        }

        value_type& operator[](const key_type& key) {
            return m_nodes[key]->get_value();
        }

        const value_type& operator[](const key_type& key) const {
            return m_nodes.at(key)->get_value();
        }

        std::unordered_map<key_type, node_type*>::iterator begin() {
            return m_nodes.begin();
        }

        std::unordered_map<key_type, node_type*>::iterator end() {
            return m_nodes.end();
        }
    private:
        void post_order_traverse_unique(
            const key_type& root,
            const function_type& function,
            std::unordered_set<key_type>& visited
        ) {
            if(visited.contains(root)) {
                return;
            }

            node_type* node = m_nodes[root];
            visited.insert(root);

            for (const auto& child : node->get_children()) {
                post_order_traverse_unique(child, function, visited);
            }

            function(root, node);
        }

        void post_order_traverse(
            const key_type& root,
            const function_type& function
        ) {
            node_type* node = m_nodes[root];

            for (const auto& child : node->get_children()) {
                post_order_traverse(child, function);
            }

            function(root, node);
        }

        bool dfs_visit(
            const key_type& path,
            std::unordered_map<key_type, bool>& visited,
            std::unordered_map<key_type, bool>& rec_stack
        ) const {
            if (!visited[path]) {
                visited[path] = true;
                rec_stack[path] = true;

                for (auto& child : m_nodes.at(path)->get_children()) {
                    if (!visited[child] && dfs_visit(child, visited, rec_stack)) {
                        return true;
                    }

                    if (rec_stack[child]) {
                        return true;
                    }
                }
            }

            rec_stack[path] = false;
            return false;
        }
    private:
        std::unordered_map<key_type, node_type*> m_nodes;
	};
}