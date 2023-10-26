#include "user.h"

namespace ir {
	void user::set_next_user(handle<user> next_user) {
		m_next_user = next_user;
	}

	handle<user> user::get_next_user() const {
		return m_next_user;
	}

	void user::set_node(handle<node> node) {
		m_node = node;
	}

	handle<node> user::get_node() const {
		return m_node;
	}

	void user::set_slot(i32 slot) {
		m_slot = slot;
	}

	i32 user::get_slot() const {
		return m_slot;
	}
}
