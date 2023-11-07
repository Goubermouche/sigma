#include "machine_block.h"

namespace ir::cg {
    machine_block::machine_block(u64 interval_count)
	    : m_gen(interval_count),
	m_kill(interval_count),
	m_live_in(interval_count),
	m_live_out(interval_count) {}

	void machine_block::set_first(handle<instruction> first) {
		m_first = first;
	}

	void machine_block::set_start(u64 start) {
		m_start = start;
	}

	void machine_block::set_end(u64 end) {
		m_end = end;
	}

	void machine_block::set_terminator(u64 terminator) {
		m_terminator = terminator;
	}

	void machine_block::set_live_in(const utility::dense_set& live_in) {
		m_live_in = live_in;
	}

	handle<instruction> machine_block::get_first() const {
		return m_first;
	}

	u64 machine_block::get_terminator() const {
		return m_terminator;
	}

	u64 machine_block::get_start() const {
		return m_start;
	}

	u64 machine_block::get_end() const {
		return m_end;
	}

	utility::dense_set& machine_block::get_gen() {
		return m_gen;
	}

	utility::dense_set& machine_block::get_kill() {
		return m_kill;
	}

	utility::dense_set& machine_block::get_live_in() {
		return m_live_in;
	}

	utility::dense_set& machine_block::get_live_out() {
		return m_live_out;
	}
}