#include "live_interval.h"
#include "intermediate_representation/codegen/codegen_context.h"

namespace ir {
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
		auto interval = this;

		// skip past previous intervals
		while (interval->split_kid >= 0 && position > interval->get_end()) {
			interval = &context.intervals[interval->split_kid];
		}

		return interval;
	}
}
