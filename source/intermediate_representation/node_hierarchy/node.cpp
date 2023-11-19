#include "node.h"
#include <utility/filesystem/file_types/text_file.h>

#include "intermediate_representation/node_hierarchy/user.h"
#include "intermediate_representation/codegen/codegen_context.h"

namespace ir {
	auto node::get_parent_region() -> handle<node> {
		handle node = this;

		while (node->ty != region && node->ty != entry) {
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
			ty == projection &&
			dt.ty == data_type::type::control &&
			inputs[0]->ty != entry
		) {
			// if it's single user and that user is the terminator we can skip it in the fallthrough logic
			return use->next_user == nullptr && use->node->ty == region ? use->node : this;
		}

		return this;
	}

	auto node::get_next_block() -> handle<node> {
		ASSERT(ty == projection && inputs[0]->ty == branch, "panic");

		if (!inputs[0]->is_critical_edge(this)) {
			return use->node;
		}

		return this;
	}

	auto node::get_node_name() const -> std::string {
		switch (ty) {
			case none: return "INVALID NODE";
			case dead: return "dead";
			case unreachable: return "unreachable";

			case entry:  return "entry";
			case exit:   return "exit";
			case projection:   return "proj";
			case region: return "region";

			case local: return "local";

			case variadic_start: return "variadic_start";
			case debug_break: return "debug_break";

			case poison: return "poison";
			case integer_constant: return "int";
			case f32_constant: return "float32";
			case f64_constant: return "float64";

			case phi: return "phi";
			case select: return "select";
			case lookup: return "lookup";

			case array_access: return "array";
			case member_access: return "member";

			case pointer_to_integer: return "ptr2int";
			case integer_to_pointer: return "int2ptr";

			case memset: return "memset";
			case memcpy: return "memcpy";

			case zero_extend: return "zxt";
			case sign_extend: return "sxt";
			case float_extend: return "fpxt";
			case truncate: return "trunc";
			case bit_cast: return "bitcast";
			case uint_to_float: return "uint2float";
			case int_to_float: return "int2float";
			case float_to_uint: return "float2uint";
			case float_to_int: return "float2int";
			case symbol: return "symbol";

			case compare_not_equal: return "cmp.ne";
			case compare_equal: return "cmp.eq";
			case compare_unsigned_less_than: return "cmp.ult";
			case compare_unsigned_less_than_or_equal: return "cmp.ule";
			case compare_signed_less_than: return "cmp.slt";
			case compare_signed_less_than_or_equal: return "cmp.sle";
			case compare_float_less_than: return "cmp.lt";
			case compare_float_less_than_or_equal: return "cmp.le";

			case clz: return "clz";
			case ctz: return "ctz";
			case negation: return "not";
			case conjunction: return "and";
			case disjunction: return "or";
			case exclusive_disjunction: return "xor";
			case addition: return "add";
			case subtraction: return "sub";
			case multiplication: return "mul";
			case unsigned_division: return "udiv";
			case signed_division: return "sdiv";
			case unsigned_modulus: return "umod";
			case signed_modulus: return "smod";
			case left_shift: return "shl";
			case right_shift: return "shr";
			case left_rotate: return "rol";
			case right_rotate: return "ror";
			case right_arithmetic_shift: return "sar";

			case float_addition: return "fadd";
			case float_subtraction: return "fsub";
			case float_multiplication: return "fmul";
			case float_division: return "fdiv";

			case multiply_pair: return "mulpair";
			case load: return "load";
			case store: return "store";

			case call: return "call";
			case system_call: return "syscall";
			case branch: 
			case tail_call: return "branch";

			default: {
				ASSERT(false, "unknown type");
				return "";
			}
		}
	}

	auto node::has_effects() const -> bool {
		switch (ty) {
			// memory effects
			case read:
			case write:
			case store:
			case memcpy:
			case memset:
				return true;
			case projection:
				return dt.ty == data_type::type::control;
			// control flow
			case entry:
			case region:
			case branch:
			case exit:
			case unreachable:
			case debug_break:
			case trap:
			case system_call:
			case call:
				return true;
			default:
				return false;
		}
	}

	auto node::is_control() const -> bool {
		// easy case
		if (dt.ty == data_type::type::control) {
			return true;
		}

		if (dt.ty != data_type::type::tuple) {
			return false;
		}

		// harder case is figuring out which tuples have control outputs (without manually
		// checking which is annoying and slow)
		//
		// branch, debug break, trap, unreachable, dead  OR  call, syscall, safe point
		return (ty >= branch && ty <= dead) || (ty >= call && ty <= safe_point_poll);
	}

	auto node::is_pinned() const -> bool {
		return ty >= entry && ty <= safe_point_poll || ty == projection;
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

		if (ty == region && predecessor->ty == projection) {
			const handle<node> parent = predecessor->inputs[0];

			// start or projections with multiple users
			if (
				parent->ty == entry || 
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
		return get_parent_region()->get<ir::region>().memory_out;
	}

	auto node::is_block_end() const -> bool {
		return ty == branch;
	}

	auto node::is_block_begin() const -> bool {
		return
			ty == region ||
			ty == projection && (inputs[0]->ty == entry || inputs[0]->ty == branch);
	}

	auto node::is_mem_out_op() const  -> bool {
		return
			dt.ty == data_type::type::memory ||
			(ty >= store && ty < atomic_cas) ||
			(ty >= call && ty <= safe_point_poll);
	}

	auto node::is_terminator() const -> bool {
		return ty == branch || ty == unreachable || ty == trap || ty == exit;
	}

	auto node::is_control_projection_node() const -> bool {
		return ty == call || ty == system_call || ty == read || ty == write;
	}

	auto node::is_critical_edge(handle<node> projection) const -> bool {
		ASSERT(projection->ty == projection, "invalid projection node");

		// multi-user proj, this means it's basically a basic block
		if (
			projection->use->next_user != nullptr ||
			projection->use->node->ty != region
		) {
			return true;
		}

		ASSERT(ty == branch, "current node is not a branch");
		const handle<node> region = projection->use->node;

		if (region->ty == region && region->inputs.get_size() > 1) {
			for (handle<user> u = region->use; u; u = u->next_user ) {
				if (u->node->ty == phi) {
					return true;
				}
			}
		}

		return false;
	}

	auto node::should_rematerialize() const -> bool {
		if (
			(ty == int_to_float || ty == integer_to_pointer) &&
			inputs[1]->ty == integer_constant
		) {
			return true;
		}

		return
			(ty == projection && inputs[0]->ty == entry) ||
			ty == f32_constant ||
			ty == f64_constant ||
			ty == integer_constant ||
			ty == member_access ||
			ty == local || 
			ty == symbol;
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

		ASSERT(false, "user system is desynchronized");
		return nullptr;
	}

	void node::add_user(handle<node> in, u64 slot, handle<user> recycled, handle<utility::block_allocator> allocator) {
		const auto new_use = recycled ? recycled : static_cast<user*>(
			allocator->allocate(sizeof(user))
		);
		 
		new_use->next_user = in->use;
		new_use->node = this;
		new_use->slot = slot;
		in->use = new_use;
	}
}
