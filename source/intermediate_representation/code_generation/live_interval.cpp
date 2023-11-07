#include "live_interval.h"

namespace ir::cg {
	range::range(u64 start, u64 end)
		: m_start(start), m_end(end) {
	}

	u64 range::get_start() const {
		return m_start;
	}

	u64 range::get_end() const {
		return m_end;
	}

	void range::set_start(u64 start) {
		m_start = start;
	}

	void range::set_end(u64 end) {
		m_end = end;
	}

	use_position::use_position(u64 position, kind kind)
		: m_position(position), m_kind(kind) {}

	u64 use_position::get_position() const {
		return m_position;
	}

	use_position::kind use_position::get_kind() const {
		return m_kind;
	}

	live_interval::live_interval(classified_reg reg, i32 data_type, u8 assigned)
		: m_start(std::numeric_limits<i32>::max()),
		m_assigned(assigned),
		m_data_type(data_type),
		m_reg(reg) {}

	reg live_interval::get_hint() const {
		return m_hint;
	}

	u64 live_interval::get_start() const {
		return m_start;
	}

	u64 live_interval::get_end() const {
		return m_end;
	}

	i32 live_interval::get_spill() const {
		return m_spill;
	}

	reg live_interval::get_assigned() const {
		return m_assigned;
	}

	classified_reg& live_interval::get_register() {
		return m_reg;
	}

	const classified_reg& live_interval::get_register() const {
		return m_reg;
	}

	std::vector<range>& live_interval::get_ranges() {
		return m_ranges;
	}

	const std::vector<range>& live_interval::get_ranges() const {
		return m_ranges;
	}

	range& live_interval::get_range(u64 index) {
		return m_ranges[index];
	}

	const range& live_interval::get_range(u64 index) const {
		return m_ranges[index];
	}

	i32 live_interval::get_active_range() const {
		return m_active_range;
	}

	i32 live_interval::get_data_type() const {
		return m_data_type;
	}

	u64 live_interval::get_use_count() const {
		return m_uses.size();
	}

	use_position& live_interval::get_use(u64 index) {
		return m_uses[index];
	}

	const use_position& live_interval::get_use(u64 index) const {
		return m_uses[index];
	}

	std::vector<use_position>& live_interval::get_uses() {
		return m_uses;
	}

	const std::vector<use_position>& live_interval::get_uses() const {
		return m_uses;
	}

	i32 live_interval::get_split_kid() const {
		return m_split_kid;
	}

	void live_interval::set_hint(reg hint) {
		m_hint = hint;
	}

	void live_interval::set_start(u64 start) {
		m_start = start;
	}

	void live_interval::set_end(u64 end) {
		m_end = end;
	}

	void live_interval::set_active_range(i32 active_range) {
		m_active_range = active_range;
	}

	void live_interval::set_assigned(reg assigned) {
		m_assigned = assigned;
	}

	void live_interval::set_spill(i32 spill) {
		m_spill = spill;
	}

	void live_interval::set_split_kid(i32 split_kid) {
		m_split_kid = split_kid;
	}

	void live_interval::set_uses(const std::vector<use_position>& uses) {
		m_uses = uses;
	}

	void live_interval::set_node(handle<node> n) {
		m_node = n;
	}

	void live_interval::add_range(const range& r) {
		ASSERT(r.get_start() <= r.get_end(), "invalid start position");
		if (!m_ranges.empty() && m_ranges.back().get_start() <= r.get_start()) {
			range& top = m_ranges.back();

			// coalesce
			top.set_start(std::min(top.get_start(), r.get_start()));
			top.set_end(std::max(top.get_end(), r.get_end()));
		}
		else {
			m_ranges.push_back(r);
		}

		if(r.get_end() > m_end) {
			m_end = r.get_end();
		}
	}

	void live_interval::add_use_position(const use_position& position) {
		m_uses.push_back(position);
	}

	void live_interval::decrement_active_range() {
		m_active_range--;
	}

	virtual_value::virtual_value() :
		m_use_count(std::numeric_limits<u64>::max()) {}

	u64 virtual_value::get_use_count() const {
		return m_use_count;
	}

	reg virtual_value::get_virtual_register() const {
		return m_virtual_register;
	}

	void virtual_value::set_use_count(u64 use_count) {
		m_use_count = use_count;
	}

	void virtual_value::set_virtual_register(reg virtual_register) {
		m_virtual_register = virtual_register;
	}

	void virtual_value::unuse() {
		m_use_count--;
	}
}