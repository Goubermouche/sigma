#pragma once
#include "intermediate_representation/node_hierarchy/node.h"

namespace sigma::ir {
	/**
	 * \brief Describes an entity which uses a given \a node at some point in
	 * it's lifetime
	 */
	struct user {
		handle<user> next_user;
		handle<node> n;
		u64 slot;
	};
}
