#include "node.h"
#include "intermediate_representation/node_hierarchy/user.h"
#include "intermediate_representation/codegen/codegen_context.h"

namespace sigma::ir {
	auto node::get_parent_region() -> handle<node> {
		handle node = this;

		while (node->ty != REGION && node->ty != ENTRY) {
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
			ty == PROJECTION &&
			data_type.ty == data_type::CONTROL &&
			inputs[0]->ty != ENTRY
		) {
			// if it's single user and that user is the terminator we can skip it in the fallthrough logic
			return use->next_user == nullptr && use->node->ty == REGION ? use->node : this;
		}

		return this;
	}

	auto node::get_next_block() -> handle<node> {
		ASSERT(ty == PROJECTION && inputs[0]->ty == BRANCH, "panic");

		if (!inputs[0]->is_critical_edge(this)) {
			return use->node;
		}

		return this;
	}

	auto node::get_node_name() const -> std::string {
		switch (ty) {
			case NONE:                      return "INVALID NODE";
			case DEAD:                      return "dead";
			case UNREACHABLE:               return "unreachable";
			case ENTRY:                     return "entry";
			case EXIT:                      return "exit";
			case PROJECTION:                return "proj";
			case REGION:                    return "region";
			case LOCAL:                     return "local";
			case VARIADIC_START:            return "variadic_start";
			case DEBUG_BREAK:               return "debug_break";
			case POISON:                    return "poison";
			case INTEGER_CONSTANT:          return "int";
			case F32_CONSTANT:              return "float32";
			case F64_CONSTANT:              return "float64";
			case PHI:                       return "phi";
			case SELECT:                    return "select";
			case LOOKUP:                    return "lookup";
			case ARRAY_ACCESS:              return "array";
			case MEMBER_ACCESS:             return "member";
			case POINTER_TO_INTEGER:        return "ptr2int";
			case INTEGER_TO_POINTER:        return "int2ptr";
			case MEMSET:                    return "memset";
			case MEMCPY:                    return "memcpy";
			case ZERO_EXTEND:               return "zxt";
			case SIGN_EXTEND:               return "sxt";
			case FLOAT_EXTEND:              return "fpxt";
			case TRUNCATE:                  return "trunc";
			case BIT_CAST:                  return "bitcast";
			case UNSIGNED_INTEGER_TO_FLOAT: return "uint2float";
			case INTEGER_TO_FLOAT:          return "int2float";
			case FLOAT_TO_UNSIGNED_INTEGER: return "float2uint";
			case FLOAT_TO_INTEGER:          return "float2int";
			case SYMBOL:                    return "symbol";
			case CMP_NE:                    return "cmp.ne";
			case CMP_EQ:                    return "cmp.eq";
			case CMP_ULT:                   return "cmp.ult";
			case CMP_ULE:                   return "cmp.ule";
			case CMP_SLT:                   return "cmp.slt";
			case CMP_SLE:                   return "cmp.sle";
			case CMP_FLT:                   return "cmp.lt";
			case CMP_FLE:                   return "cmp.le";
			case CLZ:                       return "clz";
			case CTZ:                       return "ctz";
			case NOT:                       return "not";
			case AND:                       return "and";
			case OR:                        return "or";
			case XOR:                       return "xor";
			case ADD:                       return "add";
			case SUB:                       return "sub";
			case MUL:                       return "mul";
			case UDIV:                      return "udiv";
			case DIV:                       return "sdiv";
			case UMOD:                      return "umod";
			case MOD:                       return "smod";
			case LSH:                       return "shl";
			case RSH:                       return "shr";
			case LR:                        return "rol";
			case RR:                        return "ror";
			case RAS:                       return "sar";
			case FADD:                      return "fadd";
			case FSUB:                      return "fsub";
			case FMUL:                      return "fmul";
			case FDIV:                      return "fdiv";
			case MUL_PAIR:                  return "mulpair";
			case LOAD:                      return "load";
			case STORE:                     return "store";
			case CALL:                      return "call";
			case SYSTEM_CALL:               return "syscall";
			case BRANCH: 
			case TAIL_CALL:                 return "branch";
			default: NOT_IMPLEMENTED();     return "";
		}
	}

	auto node::has_effects() const -> bool {
		switch (ty) {
			// memory effects
			case READ:
			case WRITE:
			case STORE:
			case MEMCPY:
			case MEMSET:
				return true;
			case PROJECTION:
				return data_type.ty == data_type::CONTROL;
			// control flow
			case ENTRY:
			case REGION:
			case BRANCH:
			case EXIT:
			case UNREACHABLE:
			case DEBUG_BREAK:
			case TRAP:
			case SYSTEM_CALL:
			case CALL:
				return true;
			default:
				return false;
		}
	}

	auto node::is_control() const -> bool {
		// easy case
		if (data_type.ty == data_type::CONTROL) {
			return true;
		}

		if (data_type.ty != data_type::TUPLE) {
			return false;
		}

		// harder case is figuring out which tuples have control outputs (without manually
		// checking which is annoying and slow)
		//
		// branch, debug break, trap, unreachable, dead  OR  call, syscall, safe point
		return (ty >= BRANCH && ty <= DEAD) || (ty >= CALL && ty <= SAFE_POINT_POLL);
	}

	auto node::is_pinned() const -> bool {
		return ty >= ENTRY && ty <= SAFE_POINT_POLL || ty == PROJECTION;
	}

	auto node::is_on_last_use(codegen_context& context) -> bool {
		const auto* value = context.lookup_value(this);
		return value ? value->use_count == 1 : false;
	}

	auto node::has_users(codegen_context& context) -> bool {
		const auto* value = context.lookup_value(this);
		return value ? value->virtual_register.is_valid() || value->use_count > 0 : false;
	}

	void node::use_node(codegen_context& context) {
		if (auto* value = context.lookup_value(this)) {
			value->use_count = value->use_count - 1;
		}
	}

	auto node::get_predecessor(u64 index) -> handle<node> {
		handle<node> predecessor = inputs[index];

		if (ty == REGION && predecessor->ty == PROJECTION) {
			const handle<node> parent = predecessor->inputs[0];

			// start or projections with multiple users
			if (
				parent->ty == ENTRY || 
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
			if (use->node->is_control()) {
				return use->node;
			}
		}

		return nullptr;
	}

	auto node::peek_memory() -> handle<node> {
		return get_parent_region()->get<sigma::ir::region>().memory_out;
	}

	auto node::is_block_end() const -> bool {
		return ty == BRANCH;
	}

	auto node::is_block_begin() const -> bool {
		return
			ty == REGION ||
			ty == PROJECTION && (inputs[0]->ty == ENTRY || inputs[0]->ty == BRANCH);
	}

	auto node::is_mem_out_op() const  -> bool {
		return
			data_type.ty == data_type::MEMORY ||
			(ty >= STORE && ty < ATOMIC_CAS) ||
			(ty >= CALL && ty <= SAFE_POINT_POLL);
	}

	auto node::is_terminator() const -> bool {
		return ty == BRANCH || ty == UNREACHABLE || ty == TRAP || ty == EXIT;
	}

	auto node::is_control_projection_node() const -> bool {
		return ty == CALL || ty == SYSTEM_CALL || ty == READ || ty == WRITE;
	}

	auto node::is_critical_edge(handle<node> projection) const -> bool {
		ASSERT(projection->ty == node::PROJECTION, "invalid projection node");

		// multi-user proj, this means it's basically a basic block
		if (
			projection->use->next_user != nullptr ||
			projection->use->node->ty != REGION
		) {
			return true;
		}

		ASSERT(ty == BRANCH, "current node is not a branch");
		const handle<node> region = projection->use->node;

		if (region->ty == region && region->inputs.get_size() > 1) {
			for (handle<user> u = region->use; u; u = u->next_user ) {
				if (u->node->ty == PHI) {
					return true;
				}
			}
		}

		return false;
	}

	auto node::should_rematerialize() const -> bool {
		if (
			(ty == INTEGER_TO_FLOAT || ty == INTEGER_TO_POINTER) &&
			inputs[1]->ty == INTEGER_CONSTANT
		) {
			return true;
		}

		return
			(ty == PROJECTION && inputs[0]->ty == ENTRY) ||
			ty == F32_CONSTANT ||
			ty == F64_CONSTANT ||
			ty == INTEGER_CONSTANT ||
			ty == MEMBER_ACCESS ||
			ty == LOCAL || 
			ty == SYMBOL;
	}

	auto node::is_unreachable() const -> bool {
		for (handle<user> u = use; u; u = u->next_user) {
			if (u->node->ty == UNREACHABLE) {
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
			if (old_use->slot == slot && old_use->node == this) {
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

	void node::add_user(
		handle<node> in,
		u64 slot, 
		handle<user> recycled,
		handle<utility::block_allocator> allocator
	) {
		const auto new_use = recycled ? recycled : static_cast<user*>(
			allocator->allocate(sizeof(user))
		);
		 
		new_use->next_user = in->use;
		new_use->node = this;
		new_use->slot = slot;
		in->use = new_use;
	}
} // namespace sigma::ir
