#pragma once
#include "intermediate_representation/code_generation/instruction.h"
#include <utility/containers/dense_set.h>

namespace ir::cg {
	class machine_block {
	public:
		machine_block() = default;
		machine_block(u64 interval_count);

		void set_first(handle<instruction> first);
		void set_start(i32 start);
		void set_end(i32 end);
		void set_terminator(i32 terminator);
		void set_live_in(const utility::dense_set& live_in);

		handle<instruction> get_first() const;
		i32 get_terminator() const;
		i32 get_start() const;
		i32 get_end() const;
		utility::dense_set& get_gen();
		utility::dense_set& get_kill();
		utility::dense_set& get_live_in();
		utility::dense_set& get_live_out();
	private:
		handle<instruction> m_first;
		i32 m_start = 0;
		i32 m_end = 0;
		i32 m_terminator = 0;

		utility::dense_set m_gen;
		utility::dense_set m_kill;
		utility::dense_set m_live_in;
		utility::dense_set m_live_out;
	};
}