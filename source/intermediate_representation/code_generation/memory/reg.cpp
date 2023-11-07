#include "reg.h"

namespace ir::cg {
	reg::reg() : m_id(invalid_id) {}

	reg::reg(id_type id) : m_id(id) {}

	bool reg::operator==(reg other) const {
		return m_id == other.m_id;
	}

	bool reg::is_valid() const {
		return m_id != invalid_id;
	}

	reg::id_type reg::get_id() const {
		return m_id;
	}

	void reg::set_id(id_type id) {
		m_id = id;
	}

	classified_reg::classified_reg() : m_class(invalid_class) {}

	classified_reg::classified_reg(id_type id, class_type c)
		: reg(id), m_class(c) {}

	bool classified_reg::operator==(classified_reg other) const {
		return m_id == other.m_id && m_class == other.m_class;
	}

	classified_reg::class_type classified_reg::get_class() const {
		return m_class;
	}
}
