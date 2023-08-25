#pragma once
#include "utility/memory.h"

namespace sigma {
	class operand {
	public:
		constexpr operand(u32 id, u32 u1, u32 u2)
			: m_base_id(id), m_data{ u1, u2 } {}

	private:
		u32 m_base_id;
		u32 m_data[2];
	};
	
	// registers

	class base_reg : public operand {
	public:
		//enum : u32 {
		//	bad = 0xFFu,

		//	base_signature_mask,
		//	type_non = u32(),
		//	signature = 0
		//};

		constexpr base_reg(u32 id)
			: operand(id, 0, 0) {}
	};

	class reg : public base_reg {
	public:
		constexpr reg(u32 id)
			: base_reg(id) {}
	};

	class gp : public reg {
	public:
		enum id : u32 {
			ax  = 0,
			cx  = 1,
			dx  = 2,
			bx  = 3,
			sp  = 4,
			bp  = 5,
			si  = 6,
			di  = 7,
			r8  = 8,
			r9  = 9,
			r10 = 10,
			r11 = 11,
			r12 = 12,
			r13 = 13,
			r14 = 14,
			r15 = 15
		};

		constexpr gp(u32 id)
			: reg(id) {}

	};

	class gpd : public gp {
	public:
		constexpr gpd(u32 id)
			: gp(id) {}
	};

	static constexpr gpd eax  = gpd(gp::id::ax);
	static constexpr gpd ebx  = gpd(gp::id::bx);
	static constexpr gpd ecx  = gpd(gp::id::cx);
	static constexpr gpd edx  = gpd(gp::id::dx);
	static constexpr gpd esp  = gpd(gp::id::sp);
	static constexpr gpd ebp  = gpd(gp::id::bp);
	static constexpr gpd esi  = gpd(gp::id::si);
	static constexpr gpd edi  = gpd(gp::id::di);
	static constexpr gpd r8d  = gpd(gp::id::r8);
	static constexpr gpd r9d  = gpd(gp::id::r9);
	static constexpr gpd r10d = gpd(gp::id::r10);
	static constexpr gpd r11d = gpd(gp::id::r11);
	static constexpr gpd r12d = gpd(gp::id::r12);
	static constexpr gpd r13d = gpd(gp::id::r13);
	static constexpr gpd r14d = gpd(gp::id::r14);
	static constexpr gpd r15d = gpd(gp::id::r15);
}

