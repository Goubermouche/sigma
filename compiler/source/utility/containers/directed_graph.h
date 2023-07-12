#pragma once
#include "utility/thread_pool.h"

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

        bool has_been_processed = false;
        std::mutex mutex;
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
        using function_type = std::function<void(value_type&)>;

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
                std::cout << key << '\n';

                for (const auto& dependency : node->get_children()) {
                    std::cout << "    " << dependency << '\n';
                }
            }

            std::cout << '\n';
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

        void traverse_parallel_bottom_up(
            const key_type& root,
            thread_pool& pool,
            const function_type& function
        ) {
            node_type* node = m_nodes[root];
            std::unique_lock lock{ node->mutex };
            if (node->has_been_processed) {
                return;
            }

            node->has_been_processed = true;
            if (node->get_children().empty()) {
                function(node->get_value());
                return;
            }

            std::vector<std::future<void>> threads;
            for (u64 i = 1; i < node->get_children().size(); i++) {
                threads.push_back(pool.enqueue([
                    this,
                        node,
                        i,
                        &pool,
                        function
                ] {
                        traverse_parallel_bottom_up(
                            node->get_children()[i],
                            pool,
                            function
                        );
                    }));
            }

            traverse_parallel_bottom_up(
                node->get_children()[0],
                pool,
                function
            );

            for (auto& future : threads) {
                future.get();
            }

            function(node->get_value());
        }
    private:
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