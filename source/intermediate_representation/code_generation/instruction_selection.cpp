#include "instruction_selection.h"

#include "targets/x64_target.h"

namespace ir::cg {
	void select_instructions(code_generator_context& context) {
		ASSERT(
			context.work_list.get_item(context.block_count - 1) == context.function->get_entry_node(),
			"codegen must always schedule the entry block first"
		);

		ASSERT(
			context.work_list.get_item_count() == context.block_count,
			"invalid work list"
		);

		context.work_list.clear_visited();

		// process all phi nodes first to ensure they are defined before they are used
		for(u64 i = context.block_count; i-- > 0;) {
			const handle<node> block = context.work_list.get_item(i);

			for(const handle<user> user : block->get_users()) {
				const handle<node> user_node = user->get_node();

				if (
					user_node->get_type() == node::phi &&
					user_node->get_data().get_id() != data_type::id::memory
					) {
					value_description& value = context.values[user_node->get_global_value_index()];
					value.set_use_count(std::numeric_limits<i32>::max());
					value.set_virtual_register(reg_none);
					context.work_list.visit(user_node);
				}
			}
		}

		const handle<node> exit_node = context.function->get_exit_node();
		const handle<node> exit_block = exit_node->get_parent_region();
		bool has_exit = false;

		// compile all nodes which aren't the exit node
		for (u64 i = context.block_count; i-- > 0;) {
			const handle<node> block = context.work_list.get_item(i);
			context.labels[block] = 0;

			ASSERT(
				block->get_type() == node::entry || block->get_type() == node::region,
				"invalid block type"
			);

			if(block != exit_block) {
				const handle<instruction> label_inst = create_label_instruction(context, block);
				context.fallthrough = i > 0 ? context.work_list.get_item(i - 1) : nullptr;

				if(context.fallthrough == exit_block) {
					context.fallthrough = nullptr;
				}

				if(context.first == nullptr) {
					context.first = context.head = label_inst;
				}
				else {
					append_instruction(context, label_inst);
				}

				const handle<node> end = block->get<region_property>()->end;
				select_instructions_region(context, block, end);
			}
			else {
				has_exit = true;
			}
		}

		// schedule the exit node at the end of the generated instructions 
		if(has_exit) {
			context.fallthrough = nullptr;
			const handle<instruction> label = create_label_instruction(context, exit_block);

			if(context.first == nullptr) {
				context.first = context.head = label;
			}
			else {
				append_instruction(context, label);
			}

			const handle<node> end = exit_block->get<region_property>()->end;
			select_instructions_region(context, exit_block, end);
		}
		else {
			// if there isn't an exit node, append an epilogue instruction
			append_instruction(
				context,
				context.create_instruction<empty_property>(
					instruction::epilogue,
					VOID_TYPE,
					0, 0, 0
				)
			);
		}
	}

	void select_instructions_region(
		code_generator_context& context,
		handle<node> block,
		handle<node> end
	) {
		ASSERT(
			context.work_list.get_item_count() == context.block_count,
			"invalid work list"
		);

		// logical schedule
		dfs_schedule(context, block, end);

		// define all nodes in this block
		for(u64 i = context.work_list.get_item_count(); i-- > context.block_count;) {
			const handle<node> item = context.work_list.get_item(i);

			// track the use count
			u64 use_count = 0;
			for(const handle<user> user : item->get_users()) {
				if (user->get_node()->get_input(0) != nullptr) {
					use_count++;
				}
			}

			value_description& value = context.values[item->get_global_value_index()];
			value.set_use_count(static_cast<i32>(use_count));
			value.set_virtual_register(reg_none);
		}

		// reorder phi nodes
		const u64 phi_count = context.phi_values.size();
		const handle<node> top = context.work_list.get_item(context.block_count);

		for(u64 i = 0; i < phi_count; ++i) {
			phi_value& value = context.phi_values[i];

			// mark the proper output
			value.set_destination(input_reg(context, value.get_phi()));
		}

		// handle non-memory phi nodes
		for(const handle<user> user : top->get_users()) {
			const handle<node> user_node = user->get_node();

			if (
				user_node->get_type() == node::phi &&
				user_node->get_data().get_id() != data_type::id::memory
			) {
				value_description& value = context.values[user_node->get_global_value_index()];
				const u8 destination = input_reg(context, user_node);

				// copy the phi value into a temporary
				context.phi_values.emplace_back(phi_value(nullptr, user_node, reg_none, destination));

				data_type data_type = user_node->get_data();
				const u8 tmp = allocate_virtual_register(context, nullptr, data_type);

				append_instruction(
					context,
					create_move_instruction(
						context, data_type, tmp, destination
					)
				);

				// assign the temporary value to the phi node
				value.set_virtual_register(tmp);
			}
		}

		ASSERT(
			top->get_type() == node::entry || top->get_type() == node::region,
			"invalid node type encountered"
		);

		select_instruction(context, top, reg_none);

		const handle<instruction> head = context.head;
		handle<instruction> last = nullptr;
		handle<node> previous_effect = nullptr;
		const ptr_diff start = static_cast<ptr_diff>(context.block_count) + 1;

		// process each node, allowing for potential node folding
		for(auto i = static_cast<ptr_diff>(context.work_list.get_item_count()); i-- > start;) {
			const handle<node> item = context.work_list.get_item(i);
			value_description* value = context.lookup_value(item);

			if(
				value->get_virtual_register() == reg_none &&
				item->should_rematerialize()
			) {
				continue;
			}

			// process each node and attach it to the list of generated instructions
			instruction dummy;
			dummy.set_next_instruction(nullptr);
			context.head = &dummy;

			if(
				item->get_data().get_id() == data_type::tuple ||
				item->get_data().get_id() == data_type::control ||
				item->get_data().get_id() == data_type::memory
			) {
				if(item->get_type() == node::branch) {
					// write back the phi values after a branch
					for(u64 j = 0; j < phi_count; ++j) {
						phi_value& phi = context.phi_values[j];
						data_type data_type = phi.get_phi()->get_data();
						const u8 source = input_reg(context, phi.get_node());
						hint_reg(context, phi.get_destination(), source);

						append_instruction(
							context,
							create_move_instruction(
								context, data_type, phi.get_destination(), source
							)
						);
					}
				}

				select_instruction(context, item, value->get_virtual_register());

				if(
					item->get_input(0)->get_type() == node::entry ||
					item->get_type() != node::projection
				) {
					previous_effect = item;
				}
			}
			else if(
				value->get_use_count() > 0 || 
				value->get_virtual_register() != reg_none
			) {
				if(value->get_virtual_register() == reg_none) {
					value->set_virtual_register(
						allocate_virtual_register(context, item, item->get_data())
					);
				}

				select_instruction(context, item, value->get_virtual_register());
			}

			handle<instruction> sequence_start = dummy.get_next_instruction();
			const handle<instruction> sequence_end = context.head;

			ASSERT(
				sequence_end->get_next_instruction() == nullptr,
				"invalid sequence end instruction"
			);

			if(sequence_start != nullptr) {
				if(last == nullptr) {
					last = sequence_end;
					head->set_next_instruction(dummy.get_next_instruction());
				}
				else {
					const handle<instruction> old_next = head->get_next_instruction();
					head->set_next_instruction(sequence_start);
					sequence_end->set_next_instruction(old_next);
				}
			}
		}

		// write the location details for the top effect
		if(previous_effect != nullptr) {
			// process the node and attach it to the list of generated instructions
			instruction dummy;
			dummy.set_next_instruction(nullptr);
			context.head = &dummy;

			const handle<instruction> sequence_start = dummy.get_next_instruction();
			const handle<instruction> sequence_end = context.head;

			ASSERT(
				sequence_end->get_next_instruction() == nullptr,
				"invalid sequence end instruction"
			);

			if(sequence_start != nullptr) {
				if(last == nullptr) {
					last = sequence_end;
					head->set_next_instruction(dummy.get_next_instruction());
				}
				else {
					const handle<instruction> old_next = head->get_next_instruction();
					head->set_next_instruction(sequence_start);
					sequence_end->set_next_instruction(old_next);
				}
			}
		}

		context.phi_values.clear();
		context.head = last ? last : head;
		context.work_list.get_items().resize(context.block_count);
	}

	void select_instruction(
		code_generator_context& context,
		handle<node> n, 
		u8 destination
	) {
		switch (const node::type node_type = n->get_type()) {
			case node::phi:
			case node::region: break;
			case node::entry: {
				// TODO: implement parameters

				const handle<instruction> prev = context.head;
				constexpr u8 out_count = 0;
				i32 outs[16];

				// handle known parameters
				// i32 used_gpr = 0;
				// i32 used_xmm = 0;
				// std::vector<handle<node>>& params = context.function->get_parameters();


				// insert the entry instruction (this is where parameter register come from)
				const handle<instruction> entry_inst = context.create_instruction<empty_property>(
					instruction::type::entry,
					I64_TYPE,
					out_count,
					0,
					0
				);

				memcpy(entry_inst->get_operands().get_data(), outs, out_count * sizeof(i32));
				entry_inst->set_next_instruction(prev->get_next_instruction());

				if(prev->get_next_instruction() == nullptr) {
					context.head = entry_inst;
				}

				prev->set_next_instruction(entry_inst);

				// walk the entry to find any parameter stack slots
				constexpr bool has_parameter_slots = false;

				if(has_parameter_slots) {
					context.stack_usage += 16 * context.function->get_parameter_count() * 8;
				}
				else {
					context.stack_usage += 16;
				}

				break;
			}

			case node::integer_constant: {
				u64 value = n->get<integer_property>()->value;

				// mask off bits
				const u64 bits_in_type = n->get_data().get_id() == data_type::id::pointer ? 64 : n->get_data().get_bit_width();
				if(bits_in_type < 64) {
					value &= (1ull << bits_in_type) - 1;
				}

				if(!utility::fits_into_i32(value)) {
					// append a 64-bit version of the integer
					append_instruction(
						context,
						create_abs_instruction(
							context, instruction::movabs, n->get_data(), 
							destination, value
						)
					);
				}
				else if(value == 0) {
					// append a zero instruction
					append_instruction(
						context,
						create_zero_instruction(
							context, n->get_data(), destination
						)
					);
				}
				else {
					// append a 32-bit instruction
					append_instruction(
						context,
						create_immediate_instruction(
							context, instruction::mov, n->get_data(), 
							destination, static_cast<i32>(value)
						)
					);
				}

				break;
			}

			case node::conjunction:
			case node::disjunction:
			case node::exclusive_disjunction:
			case node::addition:
			case node::subtraction: {
				constexpr static instruction::type operations[] = {
					instruction::AND,
					instruction::OR,
					instruction::XOR,
					instruction::ADD,
					instruction::SUB,
				};

				const instruction::type operation = operations[node_type - node::conjunction];
				const u8 left = input_reg(context, n->get_input(1));
				hint_reg(context, destination, left);
				i32 immediate;

				if(try_for_imm32(n->get_input(2), immediate)) {
					use(context, n->get_input(2));

					append_instruction(
						context,
						create_move_instruction(
							context, n->get_data(), destination, left
						)
					);

					append_instruction(
						context,
						create_rri_instruction(
							context, operation, n->get_data(), destination,
							destination, immediate
						)
					);
				}
				else {
					const u8 right = input_reg(context, n->get_input(2));

					append_instruction(
						context,
						create_move_instruction(
							context, n->get_data(), destination, left
						)
					);

					append_instruction(
						context,
						create_rrr_instruction(
							context, operation, n->get_data(), destination,
							destination, right
						)
					);
				}

				break;
			}

			case node::branch: {
				const handle<branch_property> branch = n->get<branch_property>();

				append_instruction(
					context,
					context.create_instruction<empty_property>(
						instruction::terminator, VOID_TYPE, 0, 0, 0
					)
				);

				if(branch->successors.size() == 1) {
					if(context.fallthrough != branch->successors[0]) {
						append_instruction(
							context,
							create_jump_instruction(context, branch->successors[0])
						);
					}
				}
				else if(branch->successors.size() == 2) {
					ASSERT(false, "not implemented - branch 2");
				}
				else {
					ASSERT(false, "not implemented - branch 2+");
				}

				break;
			}
			case node::exit: {
				if(n->get_input_count() > 3) {
					ASSERT(n->get_input_count() <= 4, "multiple returns not supported");

					const u8 source = input_reg(context, n->get_input(3));
					const data_type data_type = n->get_input(3)->get_data();

					// copy to the return register
					if (data_type.get_id() == data_type::id::floating_point) {
						ASSERT(false, "not implemented");
					}
					else {
						hint_reg(context, source, rax);

						append_instruction(
							context,
							create_move_instruction(
								context, data_type, static_cast<i32>(rax),
								source
							)
						);
					}
				}

				// we don't really need a fence if we're about to exit but we do
				// need to mark that it's the epilogue to tell the register
				// allocator where callee registers need to get restored
				append_instruction(
					context,
					context.create_instruction<empty_property>(
						instruction::epilogue,
						VOID_TYPE,
						0, 0, 0
					)
				);

				break;
			}

			case node::local:
			case node::variadic_start:
			case node::member_access:
			case node::array_access: {
				append_instruction(
					context,
					select_memory_access_instruction(
						context, n, destination, -1, -1
					)
				);

				break;
			}

			case node::store: {
				if(destination != reg_none) {
					use(context, n->get_input(2));
					use(context, n->get_input(3));
					break;
				}

				const data_type store_data_type = n->get_input(3)->get_data();
				const handle<node> address = n->get_input(2);
				handle<node> source = n->get_input(3);
				i32 immediate;

				// check if we can couple the load and the store operations
				i32 store_op = context.can_folded_store(
					n->get_input(1), 
					address,
					n->get_input(3)
				);

				if(store_op >= 0) {
					use(context, source);
					use(context, address);
					use(context, source->get_input(1));
					use(context, source->get_input(1)->get_input(1));

					source = source->get_input(2);
				}
				else {
					if(store_data_type.get_id() == data_type::id::floating_point) {
						store_op = instruction::floating_point_mov;
					}
					else {
						store_op = instruction::mov;
					}
				}

				if(try_for_imm32(source, immediate)) {
					use(context, source);

					const handle<instruction> store_inst = select_array_access_instruction(
						context, address, destination, store_op, -1
					);

					store_inst->set_in_count(store_inst->get_in_count() - 1);
					store_inst->set_data_type(context.target->legalize_data_type(store_data_type));
					store_inst->set_flags(store_inst->get_flags() | instruction::immediate);
					store_inst->set_property(context.instruction_allocator.allocate(sizeof(immediate_prop)));
					store_inst->get<immediate_prop>()->value = immediate;

					ASSERT(
						store_inst->get_flags() & (instruction::mem | instruction::global),
						"invalid store instruction flags"
					);

					append_instruction(context, store_inst);
				}
				else {
					const u8 source_reg = input_reg(context, source);
					const handle<instruction> store_inst = select_array_access_instruction(
						context, address, destination, store_op, source_reg
					);

					store_inst->set_data_type(context.target->legalize_data_type(store_data_type));

					ASSERT(
						store_inst->get_flags() & (instruction::mem | instruction::global),
						"invalid store instruction flags"
					);

					append_instruction(context, store_inst);
				}

				break;
			}

			default: {
				ASSERT(false, "not implemented");
			}
		}
	}

	handle<instruction> select_memory_access_instruction(
		code_generator_context& context, 
		handle<node> n,
		i32 destination, 
		i32 store_op, 
		i32 source
	) {
		const bool has_second_in = store_op < 0 && source >= 0;
		i64 offset = 0;
		scale scale = x1;
		i32 index = -1;
		i32 base;

		if (n->get_type() == node::symbol) {
			ASSERT(false, "not implemented");
		}
		else if (n->get_type() == node::variadic_start) {
			ASSERT(false, "not implemented");
		}
		else if (n->get_type() == node::member_access) {
			ASSERT(false, "not implemented");
		}

		if (n->get_type() == node::array_access) {
			ASSERT(false, "not implemented");
		}

		if (n->get_type() == node::local) {
			use(context, n);
			offset += get_stack_slot(context, n);
			base = static_cast<i32>(rbp);
		}
		else {
			base = input_reg(context, n);
		}

		// compute the base
		if(store_op < 0) {
			if(has_second_in) {
				return create_rrm_instruction(
					context, instruction::lea, n->get_data(), destination,
					source, base, index, scale, static_cast<i32>(offset)
				);
			}

			return create_rm_instruction(
				context, instruction::lea, n->get_data(), destination,
				base, index, scale, static_cast<i32>(offset)
			);
		}

		return create_mr_instruction(
			context, static_cast<instruction::type>(store_op), n->get_data(),
			base, index, scale, static_cast<i32>(offset), source
		);
	}

	handle<instruction> select_array_access_instruction(
		code_generator_context& context,
		handle<node> n,
		i32 destination, 
		i32 store_op,
		i32 source
	) {
		// compute base
		if(
			n->get_type() == node::array_access &&
			(context.values[n->get_global_value_index()].get_use_count() > 2 ||
			 context.values[n->get_global_value_index()].get_virtual_register() != reg_none)
		) {
			const u8 base = input_reg(context, n);

			if (store_op < 0) {
				if (source >= 0) {
					return create_rrm_instruction(
						context, instruction::lea, PTR_TYPE,
						destination, source, base, -1, x1, 0
					);
				}

				return create_rm_instruction(
					context, instruction::type::lea, PTR_TYPE,
					destination, base, -1, x1, 0
				);
			}

			return create_mr_instruction(
				context, static_cast<instruction::type>(store_op),
				PTR_TYPE, base, -1, x1, 0, source
			);
		}

		return select_memory_access_instruction(
			context, n, destination, store_op, source
		);
	}

	void dfs_schedule(
		code_generator_context& context, 
		handle<node> block, 
		handle<node> n
	) {
		// check if we are in a different block and that we haven't visited this node yet
		if(!is_same_block(block, n) || !context.work_list.visit(n)) {
			return;
		}

		// if we're in a branch, push our phi nodes
		if(n->get_type() == node::branch) {
			const handle<branch_property> b = n->get<branch_property>();

			// TODO: we can probably get away with using a node handle instead
			//       of the phi index
			for(const handle<node> dst : b->successors) {
				// find predecessor index and visit that edge
				ptr_diff phi_index = -1;
				for(ptr_diff i = 0; i < static_cast<ptr_diff>(dst->get_input_count()); ++i) {
					const handle<node> predecessor = dst->get_input(i)->get_parent_region();

					if(predecessor == block) {
						phi_index = i;
						break;
					}
				}

				if(phi_index < 0) {
					continue;
				}

				// schedule memory phis
				for(const handle<user> user : dst->get_users()) {
					const handle<node> phi = user->get_node();

					if (
						phi->get_type() == node::phi &&
						phi->get_data().get_id() == data_type::id::memory
						) {
						dfs_schedule_phi(context, block, phi, phi_index);
					}
				}

				// schedule data phis
				for(const handle<user> user : dst->get_users()) {
					const handle<node> phi = user->get_node();

					if (
						phi->get_type() == node::phi &&
						phi->get_data().get_id() != data_type::id::memory
						) {
						dfs_schedule_phi(context, block, phi, phi_index);
					}
				}
			}
		}

		// push inputs
		for(u64 i = n->get_input_count(); i-- > 0;) {
			if (const handle<node> input = n->get_input(i)) {
				dfs_schedule(context, block, input);
			}
		}

		// evaluate leftovers
		if(n->is_block_end()) {
			const handle<node> parent = n->get_block_begin();
			for(const handle<user> user : parent->get_users()) {
				dfs_schedule(context, block, user->get_node());
			}
		}

		context.work_list.add_item(n);

		if(n->is_mem_out_op()) {
			// memory effects have anti-dependencies, the previous loads
			// must finish before the next memory effect is applied
			for(const handle<user> user : n->get_input(1)->get_users()) {
				if (user->get_slot() == 1 && user->get_node() != n) {
					dfs_schedule(context, block, user->get_node());
				}
			}
		}

		// push outputs (or projections, if they apply)
		if(
			n->get_data().get_id() == data_type::id::tuple &&
			n->get_type() != node::branch
		) {
			for(const handle<user> user : n->get_users()) {
				const handle<node> user_node = user->get_node();

				if (user_node->get_type() == node::projection) {
					dfs_schedule(context, block, user_node);
				}
			}
		}
	}

	void dfs_schedule_phi(
		code_generator_context& context, 
		handle<node> block, 
		handle<node> phi,
		ptr_diff phi_index
	) {
		const handle<node> value = phi->get_input(1 + phi_index);

		// reserve phi space
		if(phi->get_data().get_id() != data_type::id::memory) {
			context.phi_values.emplace_back(phi_value{ value, phi });
		}

		dfs_schedule(context, block, value);
	}

	void append_instruction(
		code_generator_context& context,
		handle<instruction> inst
	) {
		context.head->set_next_instruction(inst);
		context.head = inst;
	}

	u8 input_reg(
		code_generator_context& context,
		handle<node> n
	) {
		// attempt to lookup an existing value
		value_description* value = context.lookup_value(n);

		// no value was found, allocate a new virtual register
		if(value == nullptr) {
			const u8 tmp = allocate_virtual_register(context, n, n->get_data());
			select_instruction(context, n, tmp);
			return tmp;
		}

		value->unuse();

		// if we have a virtual register, return its ID
		if(value->get_virtual_register() != reg_none) {
			return value->get_virtual_register();
		}

		// if the node should rematerialize we allocate a new virtual register
		if(n->should_rematerialize()) {
			const u8 tmp = allocate_virtual_register(context, n, n->get_data());
			select_instruction(context, n, tmp);
			return tmp;
		}

		// fallback to just allocating a new virtual register
		const u8 i = allocate_virtual_register(context, n, n->get_data());
		value->set_virtual_register(i);
		return i;
	}

	bool try_for_imm32(handle<node> n, i32& out_x) {
		if(n->get_type() == node::integer_constant) {
			const u64 value = n->get<integer_property>()->value;

			if(utility::fits_into_i32(value)) {
				out_x = static_cast<i32>(value);
				return true;
			}
		}

		return false;
	}

	void use(code_generator_context& context, handle<node> n) {
		if(value_description* value = context.lookup_value(n)) {
			value->set_use_count(value->get_use_count() - 1);
		}
	}

	i32 get_stack_slot(
		code_generator_context& context,
		handle<node> n
	) {
		// check if a stack slot for the node already exists 
		const auto it = context.stack_slots.find(n);

		// if it exists, return the stack position
		if(it != context.stack_slots.end()) {
			return it->second;
		}

		// allocate a new stack slot for the given node
		const handle<local_property> local = n->get<local_property>();
		const i32 position = context.allocate_stack(local->size, local->alignment);
		context.stack_slots[n] = position;
		return position;
	}

	void hint_reg(code_generator_context& context, i32 interval_index, u8 reg) {
		if(context.intervals[interval_index].get_hint() == reg_none) {
			context.intervals[interval_index].set_hint(reg);
		}
	}

	u8 allocate_virtual_register(
		code_generator_context& context, 
		handle<node> n, 
		const data_type& data_type
	) {
		const u64 index = context.intervals.size();

		// create a new live interval with an uninitialized register
		live_interval interval(
			reg(reg_none, context.target->classify_register_class(data_type)),
			context.target->legalize_data_type(data_type),
			reg_none
		);

		interval.set_node(n);
		context.intervals.emplace_back(interval);
		ASSERT(index < std::numeric_limits<u8>::max(), "invalid virtual register");
		return static_cast<u8>(index);
	}

	handle<instruction> create_rrm_instruction(
		code_generator_context& context, 
		instruction::type type, 
		const data_type& data_type,
		i32 destination,
		i32 source,
		i32 base, 
		i32 index,
		scale scale,
		i32 displacement
	) {
		const handle<instruction> inst = context.create_instruction<empty_property>(
			type, data_type, 1, index >= 0 ? 3 : 2, 0
		);

		inst->set_flags(instruction::mem | (index >= 0 ? instruction::indexed : instruction::none));
		inst->set_scale(scale);
		inst->set_displacement(displacement);
		inst->set_memory_slot(2);

		inst->set_operand(0, destination);
		inst->set_operand(1, source);
		inst->set_operand(2, base);

		if(index >= 0) {
			inst->set_operand(4, index);
		}

		return inst;
	}

	handle<instruction> create_rm_instruction(
		code_generator_context& context,
		instruction::type type, 
		const data_type& data_type, 
		i32 destination, 
		i32 base, 
		i32 index,
		scale scale, 
		i32 displacement
	) {
		const handle<instruction> inst = context.create_instruction<empty_property>(
			type, data_type, 1, index >= 0 ? 2 : 1, 0
		);

		inst->set_flags(instruction::mem | (index >= 0 ? instruction::indexed : instruction::none));
		inst->set_memory_slot(1);
		inst->set_operand(0, destination);
		inst->set_operand(1, base);

		if(index >= 0) {
			inst->set_operand(2, index);
		}

		inst->set_displacement(displacement);
		inst->set_scale(scale);
		return inst;
	}

	handle<instruction> create_mr_instruction(
		code_generator_context& context,
		instruction::type type, 
		const data_type& data_type, 
		i32 base, 
		i32 index,
		scale scale,
		i32 displacement,
		i32 source
	) {
		const handle<instruction> inst = context.create_instruction<empty_property>(
			type, data_type, 0, index >= 0 ? 3 : 2, 0
		);

		inst->set_flags(instruction::mem | (index >= 0 ? instruction::indexed : instruction::none));
		inst->set_memory_slot(0);

		inst->set_operand(0, base);

		if(index >= 0) {
			inst->set_operand(1, index);
			inst->set_operand(2, source);
		}
		else {
			inst->set_operand(1, source);
		}

		inst->set_displacement(displacement);
		inst->set_scale(scale);
		return inst;
	}

	handle<instruction> create_label_instruction(
		code_generator_context& context,
		handle<node> n
	) {
		const handle<instruction> inst = context.create_instruction<node_prop>(0);
		inst->get<node_prop>()->value = n;
		inst->set_type(instruction::label);
		inst->set_flags(instruction::node);
		return inst;
	}

	handle<instruction> create_move_instruction(
		code_generator_context& context, 
		const data_type& data_type,
		u8 destination, 
		u8 source
	) {
		const i32 machine_data_type = context.target->legalize_data_type(data_type);
		const handle<instruction> inst = context.create_instruction<empty_property>(2);

		inst->set_type(machine_data_type >= sse_ss ? instruction::floating_point_mov : instruction::mov);
		inst->set_data_type(machine_data_type);

		inst->set_out_count(1);
		inst->set_in_count(1);
		inst->set_operand(0, destination);
		inst->set_operand(1, source);

		return inst;
	}

	handle<instruction> create_abs_instruction(
		code_generator_context& context, 
		instruction::type type, 
		const data_type& data_type, 
		u8 destination,
		u64 imm
	) {
		const handle<instruction> inst = context.create_instruction<immediate_prop>(
			type, data_type, 1, 0, 0
		);

		inst->get<immediate_prop>()->value = static_cast<i32>(imm);
		inst->set_flags(instruction::absolute);
		inst->set_operand(0, destination);
		return inst;
	}

	handle<instruction> create_zero_instruction(
		code_generator_context& context,
		const data_type& data_type,
		u8 destination
	) {
		const handle<instruction> inst = context.create_instruction<empty_property>(
			instruction::zero, data_type, 1, 0, 0
		);

		inst->set_operand(0, destination);
		return inst;
	}

	handle<instruction> create_immediate_instruction(
		code_generator_context& context, 
		instruction::type type,
		const data_type& data_type,
		u8 destination,
		i32 imm
	) {
		const handle<instruction> inst = context.create_instruction<immediate_prop>(
			type, data_type, 1, 0, 0
		);

		inst->get<immediate_prop>()->value = imm;
		inst->set_flags(instruction::immediate);
		inst->set_operand(0, destination);
		return inst;
	}

	handle<instruction> create_rri_instruction(
		code_generator_context& context, 
		instruction::type type,
		const data_type& data_type, 
		u8 destination, 
		u8 source, 
		i32 imm
	) {
		const handle<instruction> inst = context.create_instruction<immediate_prop>(
			type, data_type, 1, 1, 0
		);

		inst->get<immediate_prop>()->value = imm;
		inst->set_flags(instruction::immediate);
		inst->set_operand(0, destination);
		inst->set_operand(1, source);
		return inst;
	}

	handle<instruction> create_rrr_instruction(
		code_generator_context& context,
		instruction::type type,
		const data_type& data_type, 
		u8 destination,
		u8 left,
		u8 right
	) {
		const handle<instruction> inst = context.create_instruction<immediate_prop>(
			type, data_type, 1, 2, 0
		);

		inst->set_operand(0, destination);
		inst->set_operand(1, left);
		inst->set_operand(2, right);

		return inst;
	}

	handle<instruction> create_jump_instruction(
		code_generator_context& context,
		handle<node> target
	) {
		ASSERT(
			target->get_type() > ir::node::none,
			"invalid target type for a jump instruction"
		);

		const handle<instruction> inst = context.create_instruction<node_prop>(
			instruction::jmp, VOID_TYPE, 0, 0, 0
		);

		inst->get<node_prop>()->value = target;
		inst->set_flags(instruction::node);
		return inst;
	}
}
