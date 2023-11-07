#include "mem.h"

namespace ir::cg {
	mem::mem(i32 index, scale scale, i32 displacement) :
		m_index(index), m_scale(scale), m_displacement(displacement) {}
	
	i32 mem::get_index() const {
		return m_index;
	}

	i32 mem::get_displacement() const {
		return m_displacement;
	}

	scale mem::get_scale() const {
		return m_scale;
	}
}