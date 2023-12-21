#include "live_interval.h"
#include "intermediate_representation/codegen/codegen_context.h"

namespace sigma::ir {
	auto live_interval::get_start() const -> u64 {
		return ranges.back().start;
	}

	auto live_interval::get_end() const -> u64 {
		return ranges[1].end;
	}

	void live_interval::add_range(const utility::range<u64>& range) {
		ASSERT(range.start <= range.end, "invalid start position");

		if (!ranges.empty() && ranges.back().start <= range.end) {
			utility::range<u64>& top = ranges.back();

			// coalesce
			top.start = std::min(top.start, range.start);
			top.end   = std::max(top.end, range.end);
		}
		else {
			ranges.push_back(range);
		}
	}

	auto live_interval::split_at(codegen_context& context, u64 position) -> handle<live_interval> {
		handle interval = this;

		// skip past previous intervals
		while (interval->split_child >= 0 && position > interval->get_end()) {
			interval = &context.intervals[interval->split_child];
		}

		return interval;
	}
	auto find_least_common_ancestor(handle<basic_block> a, handle<basic_block> b) -> handle<basic_block> {
		if (a == nullptr) {
			return b;
		}

		while (a->dominator_depth > b->dominator_depth) {
			a = a->dominator;
		}

		while (b->dominator_depth > a->dominator_depth) {
			b = b->dominator;
		}

		while (a != b) {
			b = b->dominator;
			a = a->dominator;
		}

		return a;
	}

	ptr_diff interval_intersect(handle<live_interval> a, handle<live_interval> b) {
		for (u64 i = a->active_range + 1; i-- > 1;) {
			for (u64 j = b->active_range + 1; j-- > 1;) {
				const ptr_diff intersect = range_intersect(a->ranges[i], b->ranges[j]);

				if (intersect >= 0) {
					return intersect;
				}
			}
		}

		return -1;
	}

} // namespace sigma::ir
