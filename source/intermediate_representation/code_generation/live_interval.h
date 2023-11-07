#pragma once
#include "intermediate_representation/nodes/node.h"
#include "intermediate_representation/code_generation/memory/reg.h"
#include "intermediate_representation/code_generation/memory/mem.h"

namespace ir::cg {
	class virtual_value {
	public:
		explicit virtual_value();

		u64 get_use_count() const;
		reg get_virtual_register() const;

		void set_use_count(u64 use_count);
		void set_virtual_register(reg virtual_register);

		void unuse();
	private:
		u64 m_use_count;
		reg m_virtual_register;
	};

	class range {
	public:
		range(u64 start, u64 end);

		u64 get_start() const;
		u64 get_end() const;

		void set_start(u64 start);
		void set_end(u64 end);
	private:
		u64 m_start;
		u64 m_end;
	};

	class use_position {
	public:
		enum kind {
			out, reg, mem_or_reg
		};

		use_position() = default;
		use_position(u64 position, kind kind);

		u64 get_position() const;
		kind get_kind() const;
	private:
		u64 m_position;
		kind m_kind;
	};

	class live_interval {
	public:
		live_interval(classified_reg reg, i32 data_type, u8 assigned);

		void set_hint(reg hint);

		void set_active_range(u64 active_range);
		void set_assigned(reg assigned);
		void set_spill(i32 spill);
		void set_split_kid(i32 split_kid);
		void set_uses(const std::vector<use_position>& uses);
		void set_node(handle<node> n);

		reg get_hint() const;
		i32 get_spill() const;
		reg get_assigned() const;
		classified_reg& get_register();
		const classified_reg& get_register() const;
		std::vector<range>& get_ranges();
		const std::vector<range>& get_ranges() const;
		range& get_range(u64 index);
		const range& get_range(u64 index) const;
		u64 get_active_range() const;
		i32 get_data_type() const;
		u64 get_use_count() const;
		use_position& get_use(u64 index);
		const use_position& get_use(u64 index) const;
		std::vector<use_position>& get_uses();
		const std::vector<use_position>& get_uses() const;
		i32 get_split_kid() const;

		void set_start(u64 start);
		void set_end(u64 end);

		u64 get_start() const;
		u64 get_end() const;

		void add_range(const range& range);
		void add_use_position(const use_position& position);
		void decrement_active_range();
	private:
		u64 m_active_range = 0;
		i32 m_data_type;
		i32 m_split_kid = -1;
		i32 m_spill = -1;

		u64 m_start = 0;
		u64 m_end = 0;

		classified_reg m_reg;
		reg m_assigned;
		reg m_hint;

		std::vector<use_position> m_uses;
		std::vector<range> m_ranges;
		handle<node> m_node;
	};
}
