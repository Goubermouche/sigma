#pragma once
#include "intermediate_representation/nodes/node.h"

namespace ir::cg {
	/**
	 * \brief Compilation work list, supports multiple operations on the underlying
	 * node tree.
	 */
	class work_list {
	public:
		/**
		 * \brief Creates a new work list by post-order-traversing the given nodes
		 * beginning with \a node. Only appends block nodes.
		 * \param node Root tree node
		 * \return New work list only containing block nodes.
		 */
		static work_list post_order(handle<node> node);

		/**
		 * \brief Traverses the entire node tree and adds all nodes inside of it
		 * into the work list.
		 * \param node Root tree node
		 */
		void push_all(handle<node> node);

		/**
		 * \brief Computes dominators for all nodes in the given node tree. 
		 * \param entry Entry node
		 * \param block_count Number of blocks to traverse
		 */
		void compute_dominators(handle<node> entry, u64 block_count) const;
		void clear();
		void clear_visited();

		u64 get_item_count() const;
		std::vector<handle<node>>& get_items();
		const std::vector<handle<node>>& get_items() const;
		handle<node> get_item(u64 index) const;
		void add_item(handle<node> n);

		/**
		 * \brief Attempts to insert \a n into the visited set.
		 * \param n Node to visit
		 * \return True if the insert operation succeeded (ie. the node was not already visited before).
		 */
		bool visit(handle<node> n);
		std::unordered_set<handle<node>>& get_visited();
		const std::unordered_set<handle<node>>& get_visited() const;
	private:
		void post_order_internal(handle<node> n);
	private:
		std::vector<handle<node>> m_items;
		std::unordered_set<handle<node>> m_visited_items;
 	};
}
