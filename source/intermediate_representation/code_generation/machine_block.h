#pragma once
#include "intermediate_representation/code_generation/instruction.h"
#include <utility/containers/dense_set.h>

// Basic machine block. Represents a block of instructions, every block also
// contains a linked list of instructions. 

namespace ir::cg {
	class machine_block {
	public:
		machine_block() = default;
		machine_block(u64 interval_count);

		void set_live_in(const utility::dense_set& live_in);
		void set_first(handle<instruction> first);
		void set_terminator(u64 terminator);

		void set_start(u64 start);
		void set_end(u64 end);

		handle<instruction> get_first() const;
		utility::dense_set& get_live_out();
		utility::dense_set& get_live_in();
		utility::dense_set& get_kill();
		utility::dense_set& get_gen();
		u64 get_terminator() const;

		u64 get_start() const;
		u64 get_end() const;
	private:
		// TODO: replace by a linked list of instructions 
		handle<instruction> m_first;
		u64 m_terminator = 0;
		u64 m_start = 0;
		u64 m_end = 0;

		utility::dense_set m_live_out;
		utility::dense_set m_live_in;
		utility::dense_set m_kill;
		utility::dense_set m_gen;
	};
}