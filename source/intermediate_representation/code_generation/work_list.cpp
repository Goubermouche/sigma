#include "work_list.h"

namespace ir::cg {
	work_list work_list::post_order(handle<node> node) {
		work_list list;
		list.post_order_internal(node);
		return list;
	}

	void work_list::push_all(handle<node> n) {
		if (visit(n)) {
			m_items.push_back(n);

			for (handle<node> input : n->get_inputs()) {
				if (input) {
					push_all(input);
				}
			}
		}
	}

	void work_list::compute_dominators(
		handle<node> entry,
		u64 block_count
	) const {
		for(u64 i = 0; i < block_count; ++i) {
			const handle<region_property> region = m_items[i]->get<region_property>();
			region->dominator_depth = -1; // unresolved
			region->dominator = nullptr;
			region->post_order_id = static_cast<i32>(i);
		}

		// entry node always dominates itself
		const handle<region_property> entry_region = entry->get<region_property>();
		entry_region->dominator_depth = 0;
		entry_region->dominator = entry;

		// identify post order traversal order
		const i64 entry_dominator = static_cast<i64>(block_count) - 1;
		bool changed = true;

		while(changed) {
			changed = false;

			// for all blocks
			for(u64 i = block_count - 1; i-- > 0;) {
				const handle<node> block = m_items[i];
				handle<node> new_immediate_dominator = block->get_input(0)->get_parent_region();

				// for all predecessors
				for(u64 j = 1; j < block->get_input_count(); ++j) {
					const handle<node> parent = block->get_input(j)->get_parent_region();
					handle<node> parent_immediate_dominator = parent->get<region_property>()->dominator;

					// check if the dominator of the parent node is already calculated
					if(
						parent_immediate_dominator == nullptr && 
						parent->get_input_count() > 0
					) {
						i64 a = parent->get<region_property>()->post_order_id;

						if(a >= 0) {
							const i64 b = new_immediate_dominator->get<region_property>()->post_order_id;

							// adjust post order ID's until they match
							while(a != b) {
								while(a < b) {
									const handle<node> dominator = m_items[a]->get_immediate_dominator();
									a = dominator ? dominator->get<region_property>()->post_order_id : entry_dominator;
								}

								while(a < b) {
									const handle<node> dominator = m_items[b]->get_immediate_dominator();
									a = dominator ? dominator->get<region_property>()->post_order_id : entry_dominator;
								}
							}

							new_immediate_dominator = m_items[a];
						}
					}
				}

				ASSERT(new_immediate_dominator != nullptr, "invalid dominator");
				const handle<region_property> block_region = block->get<region_property>();

				if(block_region->dominator != new_immediate_dominator) {
					block_region->dominator = new_immediate_dominator;
					changed = true;
				}
			}
		}

		// generate depth values for each block
		for(u64 i = 0; i < block_count - 1; ++i) {
			m_items[i]->resolve_dominator_depth();
		}
	}

	void work_list::post_order_internal(handle<node> n) {
		if(visit(n)) {
			const handle<region_property> region = n->get<region_property>();

			// walk control edges (predecessors)
			if(region->end->get_type() == node::branch) {
				const handle<branch_property> branch = region->end->get<branch_property>();

				for (u64 i = branch->successors.size(); i-- > 0;) {
					post_order_internal(branch->successors[i]);
				}
			}

			m_items.emplace_back(n);
		}
	}

	void work_list::clear() {
		m_visited_items.clear();
		m_items.clear();
	}

	void work_list::clear_visited() {
		m_visited_items.clear();
	}

	u64 work_list::get_item_count() const {
		return m_items.size();
	}

	std::vector<handle<node>>& work_list::get_items() {
		return m_items;
	}

	const std::vector<handle<node>>& work_list::get_items() const {
		return m_items;
	}

	handle<node> work_list::get_item(u64 index) const {
		return m_items[index];
	}

	void work_list::add_item(handle<node> n) {
		m_items.push_back(n);
	}

	bool work_list::visit(handle<node> n) {
		return m_visited_items.insert(n).second;
	}

	std::unordered_set<handle<node>>& work_list::get_visited() {
		return m_visited_items;
	}

	const std::unordered_set<handle<node>>& work_list::get_visited() const {
		return m_visited_items;
	}
}
