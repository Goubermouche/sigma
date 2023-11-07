#include "phi_value.h"

namespace ir::cg {
	phi_value::phi_value(
		handle<node> n,
		handle<node> phi,
		reg source,
		reg destination
	) : m_node(n),
		m_phi(phi),
		m_source(source),
		m_destination(destination) {}

	void phi_value::set_node(handle<node> node) {
		m_node = node;
	}

	void phi_value::set_phi(handle<node> phi) {
		m_phi = phi;
	}

	void phi_value::set_source(reg source) {
		m_source = source;
	}

	void phi_value::set_destination(reg destination) {
		m_destination = destination;
	}

	handle<node> phi_value::get_phi() const {
		return m_phi;
	}

	handle<node> phi_value::get_node() const {
		return m_node;
	}

	reg phi_value::get_destination() const {
		return m_destination;
	}
}
