#pragma once
#include "intermediate_representation/nodes/node.h"

namespace ir::cg {
	enum class register_class {
		gpr,
		xmm,
		first_gpr = 0,
		first_xmm = 16
	};

	enum scale : i8 {
		x1,
		x2,
		x4,
		x8
	};

	enum class gpr {
		rax,
		rcx,
		rdx,
		rbx,
		rsp,
		rbp,
		rsi,
		rdi,
		r8,
		r9,
		r10,
		r11,
		r12,
		r13,
		r14,
		r15,
		none = -1
	};

	enum class xmm {
		xmm0,
		xmm1,
		xmm2,
		xmm3,
		xmm4,
		xmm5,
		xmm6,
		xmm7,
		xmm8,
		xmm9,
		xmm10,
		xmm11,
		xmm12,
		xmm13,
		xmm14,
		xmm15,
		none = -1
	};

	class value_description {
	public:
		i32 get_use_count() const;
		i32 get_virtual_register() const;

		void set_use_count(i32 use_count);
		void set_virtual_register(i32 virtual_register);

		void unuse();
	private:
		i32 m_use_count;
		i32 m_virtual_register;
	};

	class range {
	public:
		range(i32 start, i32 end);
		i32 get_start() const;
		i32 get_end() const;
		void set_start(i32 start);
		void set_end(i32 end);
	private:
		i32 m_start;
		i32 m_end;
	};

	class use_position {
	public:
		enum kind {
			out,
			reg,
			mem_or_reg
		};

		use_position() = default;
		use_position(i32 position, kind kind);

		i32 get_position() const;
		kind get_kind() const;
	private:
		i32 m_position;
		kind m_kind;
	};

	class reg {
	public:
		reg(i32 id, i32 reg_class);

		i32 get_id() const;
		i32 get_class() const;

		void set_id(i32 id);
		void set_class(i32 reg_class);
	private:
		i32 m_id;
		i32 m_class;
	};

	class live_interval {
	public:
		live_interval(reg reg, i32 data_type, i32 assigned);

		void set_hint(i32 hint);
		void set_start(i32 start);
		void set_end(i32 end);
		void set_active_range(i32 active_range);
		void set_assigned(i32 assigned);
		void set_spill(i32 spill);
		void set_split_kid(i32 split_kid);
		void set_uses(const std::vector<use_position>& uses);
		void set_node(handle<node> n);

		i32 get_hint() const;
		i32 get_start() const;
		i32 get_end() const;
		i32 get_spill() const;
		i32 get_assigned() const;
		reg& get_register();
		const reg& get_register() const;
		std::vector<range>& get_ranges();
		const std::vector<range>& get_ranges() const;
		range& get_range(u64 index);
		const range& get_range(u64 index) const;
		i32 get_active_range() const;
		i32 get_data_type() const;
		u64 get_use_count() const;
		use_position& get_use(u64 index);
		const use_position& get_use(u64 index) const;
		std::vector<use_position>& get_uses();
		const std::vector<use_position>& get_uses() const;
		i32 get_split_kid() const;

		void add_range(const range& range);
		void add_use_position(const use_position& position);
		void decrement_active_range();
	private:
		i32 m_start = 0;
		i32 m_end = 0;
		i32 m_assigned;
		i32 m_spill = -1;
		i32 m_active_range = 0;
		i32 m_split_kid = -1;
		i32 m_hint = -1;
		i32 m_data_type;

		reg m_reg;
		handle<node> m_node;
		std::vector<range> m_ranges;
		std::vector<use_position> m_uses;
	};
}
