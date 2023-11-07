#pragma once
#include<utility/types.h>

namespace ir::cg {
	using namespace utility::types;

	enum class scale : u8 {
		x1, x2, x4, x8
	};

	class mem {
	public:
		mem(i32 index, scale scale, i32 displacement);

		scale get_scale() const;

		i32 get_index() const;
		i32 get_displacement() const;
	private:
		scale m_scale;

		i32 m_index;
		i32 m_displacement;
	};
}
