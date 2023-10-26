#pragma once
#include "intermediate_representation/nodes/node.h"

namespace ir {
	/**
	 * \brief Describes an entity which uses a given \a node at some point in
	 * it's lifetime
	 */
	class user {
	public:
		void set_next_user(handle<user> next_user);
		handle<user> get_next_user() const;

		void set_node(handle<node> node);
		handle<node> get_node() const;

		void set_slot(i32 slot);
		i32 get_slot() const;
	private:
		handle<user> m_next_user;
		handle<node> m_node;
		i32 m_slot;
	};
}
