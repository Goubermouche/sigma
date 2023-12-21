#include "node.h"
#include "intermediate_representation/node_hierarchy/user.h"
#include "intermediate_representation/codegen/codegen_context.h"

namespace sigma::ir {
	node::type::type() : m_type(NONE) {}
	node::type::type(underlying type) : m_type(type) {}

	node::type::operator underlying() const {
		return m_type;
	}

	auto node::get_parent_region() -> handle<node> {
		handle node = this;

		while (node != type::REGION && node != type::ENTRY) {
			ASSERT(node->inputs[0] != nullptr, "node has no control edge");
			node = node->inputs[0];
		}

		return node;
	}

	auto node::get_block_entry() -> handle<node> {
		handle n = this;

		while(!n->is_block_begin()) {
			n = n->inputs[0];
		}

		return n;
	}

	auto node::get_fallthrough() -> handle<node> {
		if(
			m_type == type::PROJECTION &&
			dt == data_type::base::CONTROL &&
			inputs[0] != type::ENTRY
		) {
			// if it's single user and that user is the terminator we can skip it in the fallthrough logic
			return use->next_user == nullptr && use->target == type::REGION ? use->target : this;
		}

		return this;
	}

	auto node::get_next_block() -> handle<node> {
		ASSERT(m_type == type::PROJECTION && inputs[0] == type::BRANCH, "panic");

		if (!inputs[0]->is_critical_edge(this)) {
			return use->target;
		}

		return this;
	}

	auto node::get_type() const -> type {
		return m_type;
	}

	void node::set_type(type type) {
		m_type = type;
	}

	auto node::has_effects() const -> bool {
		switch (m_type) {
			// memory effects
			case type::READ:
			case type::WRITE:
			case type::STORE:
			case type::MEMCPY:
			case type::MEMSET:
				return true;
			case type::PROJECTION:
				return dt == data_type::base::CONTROL;
			// control flow
			case type::ENTRY:
			case type::REGION:
			case type::BRANCH:
			case type::EXIT:
			case type::UNREACHABLE:
			case type::DEBUG_BREAK:
			case type::TRAP:
			case type::SYSTEM_CALL:
			case type::CALL:
				return true;
			default:
				return false;
		}
	}

	auto node::is_control() const -> bool {
		// easy case
		if (dt == data_type::base::CONTROL) {
			return true;
		}

		if (dt != data_type::base::TUPLE) {
			return false;
		}

		// harder case is figuring out which tuples have control outputs (without manually
		// checking which is annoying and slow)
		//
		// branch, debug break, trap, unreachable, dead  OR  call, syscall, safe point
		return
			(m_type >= type::BRANCH && m_type <= type::DEAD) ||
			(m_type >= type::CALL && m_type <= type::SAFE_POINT_POLL);
	}

	auto node::is_pinned() const -> bool {
		return
			(m_type >= type::ENTRY && m_type <= type::SAFE_POINT_POLL) || 
			m_type == type::PROJECTION;
	}

	auto node::is_on_last_use(codegen_context& context) -> bool {
		const handle<virtual_value> value = context.lookup_virtual_value(this);
		return value ? value->use_count == 1 : false;
	}

	auto node::has_users(codegen_context& context) -> bool {
		const handle<virtual_value> value = context.lookup_virtual_value(this);
		return value ? value->virtual_register.is_valid() || value->use_count > 0 : false;
	}

	void node::use_node(codegen_context& context) {
		if (const handle<virtual_value> value = context.lookup_virtual_value(this)) {
			value->use_count = value->use_count - 1;
		}
	}

	auto node::get_predecessor(u64 index) -> handle<node> {
		handle<node> predecessor = inputs[index];

		if (m_type == type::REGION && predecessor == type::PROJECTION) {
			const handle<node> parent = predecessor->inputs[0];

			// start or projections with multiple users
			if (
				parent == type::ENTRY ||
				(!parent->is_control_projection_node() && predecessor->use->next_user != nullptr)
			) {
				return predecessor;
			}

			predecessor = parent;
		}

		while (!predecessor->is_block_begin()) {
			predecessor = predecessor->inputs[0];
		}

		return predecessor;
	}

	auto node::get_next_control() const -> handle<node> {
		for (auto u = use; u; u = u->next_user) {
			if (use->target->is_control()) {
				return use->target;
			}
		}

		return nullptr;
	}

	auto node::peek_memory() -> handle<node> {
		return get_parent_region()->get<sigma::ir::region>().memory_out;
	}

	auto node::is_block_end() const -> bool {
		return m_type == type::BRANCH;
	}

	auto node::is_block_begin() const -> bool {
		return
			m_type == type::REGION ||
			(m_type == type::PROJECTION && (inputs[0]->m_type == type::ENTRY || inputs[0]->m_type == type::BRANCH));
	}

	auto node::is_mem_out_op() const  -> bool {
		return
			dt == data_type::base::MEMORY ||
			(m_type >= type::STORE && m_type < type::ATOMIC_CAS) ||
			(m_type >= type::CALL && m_type <= type::SAFE_POINT_POLL);
	}

	auto node::is_terminator() const -> bool {
		return
			m_type == type::BRANCH ||
			m_type == type::UNREACHABLE ||
			m_type == type::TRAP ||
			m_type == type::EXIT;
	}

	auto node::is_control_projection_node() const -> bool {
		return
			m_type == type::CALL ||
			m_type == type::SYSTEM_CALL || 
			m_type == type::READ ||
			m_type == type::WRITE;
	}

	auto node::is_critical_edge(handle<node> projection) const -> bool {
		ASSERT(projection == type::PROJECTION, "invalid projection node");

		// multi-user proj, this means it's basically a basic block
		if (
			projection->use->next_user != nullptr ||
			projection->use->target != type::REGION
		) {
			return true;
		}

		ASSERT(m_type == type::BRANCH, "current node is not a branch");
		const handle<node> region = projection->use->target;

		if (region == type::REGION && region->inputs.get_size() > 1) {
			for (handle<user> u = region->use; u; u = u->next_user ) {
				if (u->target->m_type == type::PHI) {
					return true;
				}
			}
		}

		return false;
	}

	auto node::should_rematerialize() const -> bool {
		if (
			(m_type == type::INTEGER_TO_FLOAT || m_type == type::INTEGER_TO_POINTER) &&
			inputs[1]->m_type == type::INTEGER_CONSTANT
		) {
			return true;
		}

		return
			(m_type == type::PROJECTION && inputs[0]->m_type == type::ENTRY) ||
			m_type == type::F32_CONSTANT ||
			m_type == type::F64_CONSTANT ||
			m_type == type::INTEGER_CONSTANT ||
			m_type == type::MEMBER_ACCESS ||
			m_type == type::LOCAL || 
			m_type == type::SYMBOL;
	}

	auto node::is_unreachable() const -> bool {
		for (handle<user> u = use; u; u = u->next_user) {
			if (u->target == type::UNREACHABLE) {
				return true;
			}
		}

		return false;
	}

	auto node::remove_user(u64 slot) -> handle<user> {
		// early out: there was no previous input
		if (inputs[slot] == nullptr) {
			return nullptr;
		}
		
		const handle<node> old = inputs[slot];
		handle<user> old_use = old->use;

		if (old_use == nullptr) {
			return nullptr;
		}

		// remove the old user
		for (handle<user> prev = nullptr; old_use; prev = old_use, old_use = old_use->next_user) {
			if (old_use->slot == slot && old_use->target == this) {
				// remove the given node
				if (prev != nullptr) {
					prev->next_user = old_use->next_user;
				}
				else {
					old->use = old_use->next_user;
				}

				return old_use;
			}
		}

		PANIC("user system is desynchronized");
		return nullptr;
	}

	void node::add_user(handle<node> in, u64 slot, handle<user> recycled, handle<utility::block_allocator> allocator) {
		const auto new_use = recycled ? recycled : static_cast<user*>(
			allocator->allocate(sizeof(user))
		);
		 
		new_use->next_user = in->use;
		new_use->target = this;
		new_use->slot = slot;
		in->use = new_use;
	}

	bool operator==(handle<node> target, node::type::underlying type) {
		return target->get_type() == type;
	}

} // namespace sigma::ir
