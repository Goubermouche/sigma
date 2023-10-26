#include "symbol.h"

namespace ir {
	symbol::symbol(const std::string& name, tag tag)
		: m_name(name), m_tag(tag) {}

	const std::string& symbol::get_name() const {
		return m_name;
	}

	void symbol::set_name(const std::string& name) {
		m_name = name;
	}

	symbol::tag symbol::get_tag() const {
		return m_tag;
	}

	void symbol::set_tag(tag tag) {
		m_tag = tag;
	}
}