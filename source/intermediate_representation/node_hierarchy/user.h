#pragma once
#include "intermediate_representation/node_hierarchy/node.h"

namespace ir {
	/**
	 * \brief Describes an entity which uses a given \a node at some point in
	 * it's lifetime
	 */
	struct user {
		handle<user> next_user;
		handle<node> node;
		u64 slot;
	};
}