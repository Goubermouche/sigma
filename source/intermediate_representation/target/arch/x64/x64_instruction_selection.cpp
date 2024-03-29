#include "x64.h"
#include <abstract_syntax_tree/node.h>

#include "intermediate_representation/target/system/systemv/systemv.h"
#include "intermediate_representation/target/system/win/win.h"
#include "intermediate_representation/codegen/instruction.h"

namespace sigma::ir {
	void x64_architecture::select_instructions(codegen_context& context) {
		ASSERT(context.work.items.size() == context.graph.blocks.size(), "misaligned control flow graph detected");

		context.basic_block_order.reserve(context.graph.blocks.size());
		u64 stop_block;

		for (u64 i = 0; i < context.graph.blocks.size(); ++i) {
			handle<node> basic_block = context.work.items[i];

			for (handle<user> use = basic_block->use; use; use = use->next_user) {
				const handle<node> use_node = use->target;

				if (use_node == node::type::PHI && use_node->dt != data_type::base::MEMORY) {
					context.virtual_values[use_node->global_value_index] = virtual_value();
					context.work.visit(use_node);
				}
			}

			const handle<node> basic_block_end = context.graph.blocks.at(basic_block).end;

			if (basic_block_end == node::type::EXIT) {
				stop_block = i;
			}
			else {
				context.basic_block_order.push_back(i);
			}
		}

		context.basic_block_order.push_back(stop_block);

		for (u64 i = 0; i < context.basic_block_order.size(); ++i) {
			handle<node> basic_block = context.work.items[context.basic_block_order[i]];
			const handle<instruction> label = create_label(context, basic_block);

			context.labels[context.basic_block_order[i]] = 0;

			if(i + 1 < context.basic_block_order.size()) {
				context.fallthrough = context.basic_block_order[i + 1];
			}
			else {
				context.fallthrough = std::numeric_limits<u64>::max();
			}

			if(context.first) {
				context.append_instruction(label);
			}
			else {
				context.first = context.head = label;
			}

			const handle<node> block_end = context.graph.blocks.at(basic_block).end;
			select_instructions_region(context, basic_block, block_end, i);
		}
	}

	void x64_architecture::select_instructions_region(codegen_context& context, handle<node> block_entry, handle<node> block_end, u64 rpo_index) {
		ASSERT(context.work.items.size() == context.graph.blocks.size(), "invalid work list");
		handle<basic_block> block = context.schedule.at(block_entry);

		// logical schedule
		dfs_schedule(context, block, block_end, true);

		// schedule params
		if (rpo_index == 0) {
			for (auto use = context.function->entry_node->use; use; use = use->next_user) {
				handle<node> user_node = use->target;

				if (user_node == node::type::PROJECTION && context.work.visit(user_node)) {
					context.work.items.push_back(user_node);
				}
			}
		}

		// define all the nodes in this block
		for (u64 i = context.graph.blocks.size(); i < context.work.items.size(); ++i) {
			handle<node> block_node = context.work.items[i];
			u64 use_count = 0;

			// track non-dead users
			for (auto use = block_node->use; use; use = use->next_user) {
				if (context.schedule.contains(use->target)) {
					use_count++;
				}
			}

			context.virtual_values[block_node->global_value_index].use_count = use_count;
		}

		// phi nodes within this block should view themselves as the previous value,
		// and not the one we're producing
		u64 old_phi_count = context.phi_values.get_size();

		for (u64 i = 0; i < old_phi_count; ++i) {
			auto& phi = context.phi_values[i];

			// mark the proper output, especially before we make the BB-local ones
			phi.destination = allocate_node_register(context, phi.phi);
		}

		if (block_entry == node::type::REGION) {
			for (handle<user> user = block_entry->use; user; user = user->next_user) {
				handle<node> user_node = user->target;

				if (user_node == node::type::PHI && user_node->dt != data_type::base::MEMORY) {
					// copy the phi into a temporary
					const phi_value phi{
						.phi = user_node,
						.destination = allocate_node_register(context, user_node),
					};

					context.phi_values.push_back(phi);

					auto dt = phi.phi->dt;
					reg tmp = allocate_virtual_register(context, nullptr, dt);

					context.append_instruction(create_move(context, dt, tmp, phi.destination));
					context.virtual_values.at(user_node->global_value_index).virtual_register = tmp;
				}
			}
		}

		if (rpo_index == 0) {
			select_instruction(context, context.function->entry_node, reg::invalid_id);
		}

		// walk all nodes (we're allowed to fold nodes into those which appear later)
		// instruction selection is emitting start->end but we're iterating in reverse
		// order so we need to reverse the instruction stream as we go, it's a linked
		// list so it's not hard.
		handle<instruction> head = context.head;
		handle<instruction> last = nullptr;
		handle<node> prev_effect = nullptr;

		for (u64 i = context.work.items.size(); i-- > context.graph.blocks.size();) {
			handle<node> target = context.work.items[i];

			if(target == node::type::ENTRY) {
				continue;
			}

			handle<virtual_value> value = context.lookup_virtual_value(target);

			if(target != block_end && value->virtual_register.is_valid() == false && target->should_rematerialize()) {
				continue;
			}

			// attach to dummy list
			instruction dummy;
			dummy.next_instruction = nullptr;
			dummy.set_type(static_cast<instruction::type::underlying>(9999));
			context.head = &dummy;

			if (target != node::type::MUL_PAIR && (target->dt == data_type::base::TUPLE || target->dt == data_type::base::CONTROL || target->dt == data_type::base::MEMORY)) {
				if (target == node::type::BRANCH) {
					ASSERT(old_phi_count == 0, "branches don't get phi edges, they should've been split");
				}

				select_instruction(context, target, value->virtual_register);

				if ((target->inputs.get_size() > 0 && target->inputs[0] == node::type::ENTRY) || target != node::type::PROJECTION) {
					if (prev_effect != nullptr) {
						// set location
					}

					prev_effect = target;
				}
			}
			else if (value->use_count > 0 || value->virtual_register.is_valid()) {
				if (value->virtual_register.is_valid() == false) {
					value->virtual_register = allocate_virtual_register(context, target, target->dt);
				}

				select_instruction(context, target, value->virtual_register);
			}

			handle<instruction> seq_start = dummy.next_instruction;
			handle<instruction> seq_end = context.head;

			ASSERT(seq_end->next_instruction == nullptr, "sequence end contains subsequent nodes");

			if (seq_start != nullptr) {
				if (last == nullptr) {
					last = seq_end;
					head->next_instruction = dummy.next_instruction;
				}
				else {
					handle<instruction> old_next = head->next_instruction;
					head->next_instruction = seq_start;
					seq_end->next_instruction = old_next;
				}
			}
		}

		// write location for the top effect
		if (prev_effect) {
			instruction dummy;
			dummy.next_instruction = nullptr;
			context.head = &dummy;

			handle<instruction> seq_start = dummy.next_instruction;
			handle<instruction> seq_end = context.head;
			ASSERT(seq_end->next_instruction == nullptr, "panic");

			if (seq_start != nullptr) {
				if (last == nullptr) {
					last = seq_end;
					head->next_instruction = dummy.next_instruction;
				}
				else {
					handle<instruction> old_next = head->next_instruction;
					head->next_instruction = seq_start;
					seq_end->next_instruction = old_next;
				}
			}
		}

		// restore the PHI value to normal
		for (u64 i = old_phi_count; i < context.phi_values.get_size(); ++i) {
			auto& value = context.phi_values[i];
			context.lookup_virtual_value(value.phi)->virtual_register = value.destination;
		}

		context.phi_values.set_size(0);
		context.head = last ? last : head;

		if (block_end != node::type::EXIT && block_end != node::type::TRAP && block_end != node::type::BRANCH && block_end != node::type::UNREACHABLE) {
			// implicit goto
			handle<node> successor = block_end->get_next_control();
			context.append_instruction(create_instruction(context, instruction::type::TERMINATOR, VOID_TYPE, 0, 0, 0));

			// reset phi's
			for (u64 i = 0; i < old_phi_count; ++i) {
				auto& phi = context.phi_values[i];

				auto dt = phi.phi->dt;
				reg src = allocate_node_register(context, phi.target);

				context.hint_reg(phi.destination.id, src);
				context.append_instruction(create_move(context, dt, phi.destination, src));
			}

			u64 successor_id = context.graph.blocks.at(successor).id;

			if (context.fallthrough != successor_id) {
				context.append_instruction(create_jump(context, successor_id));
			}
		}

		context.work.items.resize(context.graph.blocks.size());
	}

	void x64_architecture::select_instruction(codegen_context& context, handle<node> n, reg destination) {
		switch (const node::type node_type = n->get_type()) {
			case node::type::PHI:
			case node::type::REGION: break;
			case node::type::ENTRY: {
				bool is_systemv = context.target.get_abi() == abi::SYSTEMV;
				const x64::gpr* gpr_params = is_systemv ? systemv::g_parameters : win::g_parameters;

				u8 gpr_param_count = 4;
				u8 xmm_param_count = is_systemv ? 8 : 4;

				handle<instruction> prev = context.head;

				u8 out_count = 0;
				i32 outs[16];

				// handle known parameters
				i32 used_gpr = 0;
				i32 used_xmm = 0;
				auto& params = context.function->parameters;

				for (u64 i = 0; i < context.function->parameter_count; ++i) {
					handle<node> projection = params[3 + i];
					bool is_float = projection->dt == data_type::base::FLOAT;
					i32 reg_limit = is_float ? xmm_param_count : gpr_param_count;
					i32 id = is_float ? used_xmm : used_gpr;

					if (is_systemv) {
						if (is_float) {
							used_xmm++;
						}
						else {
							used_gpr++;
						}
					}
					else {
						// win64 will expend the slot regardless of if it's used
						used_gpr++;
						used_xmm++;
					}

					if (id < reg_limit) {
						auto value = context.lookup_virtual_value(projection);

						if (value != nullptr) {
							ASSERT(value->virtual_register.is_valid() == false, "unexpected valid register");
							value->virtual_register = allocate_virtual_register(
								context, projection, projection->dt
							);

							i32 reg_num = is_float ? id : static_cast<i32>(gpr_params[id]);
							i32 virtual_reg = (is_float ? x64::register_class::FIRST_XMM : 0) + reg_num;

							context.hint_reg(value->virtual_register.id, static_cast<u8>(virtual_reg));

							context.append_instruction(create_move(
								context, 
								projection->dt, 
								value->virtual_register,
								static_cast<u8>(virtual_reg)
							));

							outs[out_count++] = virtual_reg;
						}
					}
				}

				// insert INST_ENTRY (this is where parameter register come from)
				handle<instruction> entry_inst = create_instruction(
					context, instruction::type::ENTRY, VOID_TYPE, out_count, 0, 0
				);

				memcpy(entry_inst->operands.get_data(), outs, out_count * sizeof(i32));

				entry_inst->next_instruction = prev->next_instruction;

				if (prev->next_instruction == nullptr) {
					context.head = entry_inst;
				}

				prev->next_instruction = entry_inst;
				bool has_param_slots = false;

				// walk the entry to find any parameter stack slots
				for (u64 i = 0; i < context.function->parameter_count; ++i) {
					handle<node> projection = params[3 + i];
					
					handle<user> user = projection->use;
					if (user == nullptr || user->next_user != nullptr || user->slot == 0) {
						continue;
					}

					handle<node> store = user->target;
					if (store != node::type::STORE || store->inputs[0]->get_parent_region() != n) {
						continue;
					}

					handle<node> address = store->inputs[2];
					if (address != node::type::LOCAL) {
						continue;
					}

					u64 pos = 16 + i * 8;

					context.stack_slots[address] = static_cast<i32>(pos);

					if (i >= 4 && context.target.get_abi() == abi::WIN_64) {
						if (handle<virtual_value> value = context.lookup_virtual_value(store)) {
							value->virtual_register = 0;
						}
					}

					has_param_slots = true;
				}

				if (has_param_slots) {
					context.stack_usage += 16 + context.function->parameter_count * 8;
				}
				else {
					context.stack_usage += 16;
				}

				// TODO: handle varargs
				break;
			}

			case node::type::TRUNCATE: {
				reg src = allocate_node_register(context, n->inputs[1]);

				if(n->dt.get_base().get_underlying() == data_type::base::FLOAT) {
					context.append_instruction(create_rr(context, instruction::type::FP_CVT, n->inputs[1]->dt, destination, src));
				}
				else {
					context.append_instruction(create_move(context, n->dt, destination, src));
				}

				break;
			}

			case node::type::SIGN_EXTEND:
			case node::type::ZERO_EXTEND: {
				handle<node> src = n->inputs[1];
				data_type src_dt = src->dt;
				bool sign_ext = node_type == node::type::SIGN_EXTEND;
				i32 bits_in_type = src_dt.get_base().get_underlying() == data_type::base::POINTER ? 64 : src_dt.get_bit_width();
				i32 imm;

				if(try_for_imm32(src, bits_in_type, imm)) {
					#define MASK_UPTO(pos) (~UINT64_C(0) >> (64 - (pos)))
					src->use_node(context);

					u64 src_value = imm;
					u64 sign_bit = (src_value >> (bits_in_type - 1)) & 1;
					u64 mask = MASK_UPTO(64) & ~MASK_UPTO(bits_in_type);

					src_value = (src_value & ~mask) | (sign_bit ? mask : 0);
					if(!utility::fits_into_32_bits(src_value)) {
						context.append_instruction(create_abs(context, instruction::type::MOVABS, n->dt, destination, src_value));
					}
					else {
						context.append_instruction(create_immediate(context, instruction::type::MOV, n->dt, destination, static_cast<i32>(src_value)));
					}

					#undef MASK_UPTO
				}
				else {
					data_type dt = n->dt;
					instruction::type op = instruction::type::MOV;

					if (bits_in_type <= 8) {
						op = sign_ext ? instruction::type::MOVSXB : instruction::type::MOVZXB;
					}
					else if (bits_in_type <= 16) {
						op = sign_ext ? instruction::type::MOVSXW : instruction::type::MOVZXW;
					}
					else if (bits_in_type <= 32) {
						if (sign_ext) {
							op = instruction::type::MOVSXD;
						}
						else {
							dt = src_dt;
						}
					}
					else if (bits_in_type <= 64) {
						op = instruction::type::MOV;
					}
					else {
						NOT_IMPLEMENTED();
					}

					reg val = allocate_node_register(context, src);
					context.append_instruction(create_rr(context, op, dt, destination, val));
				}

				break;
			}

			case node::type::SYSTEM_CALL:
			case node::type::CALL: {
				bool is_systemv = context.target.get_abi() == abi::SYSTEMV;
				static reg default_return_registers[2] = {
					static_cast<reg::id_type>(x64::gpr::RAX), static_cast<reg::id_type>(x64::gpr::RDX)
				};

				const parameter_descriptor descriptor = context.target.get_parameter_descriptor();

				if (node_type == node::type::SYSTEM_CALL) {
					NOT_IMPLEMENTED();
				}

				u32 caller_saved_gp_registers = descriptor.caller_saved_gpr_count;
				u32 caller_saved_xmm_registers = ~0ull >> (64 - descriptor.caller_saved_xmm_count);
				const function_signature& callee_signature = n->get<function_call>().signature;

				reg return_registers[2] = { reg::invalid_id, reg::invalid_id };
				handle<node> return_nodes[2] = { nullptr };
				u8 return_count = 0;

				ASSERT(callee_signature.returns.size() <= 2, "invalid function return count");

				for (u64 i = 0; i < callee_signature.returns.size(); ++i) {
					handle<node> return_node = n->get<function_call>().projections[2 + i];

					if(return_node && !return_node->has_users(context)) {
						return_node = nullptr;
					}

					if (return_node) {
						return_nodes[i] = return_node;
						return_registers[i] = allocate_node_register(context, return_node);
						return_count++;

						if (return_node->dt == data_type::base::FLOAT) {
							caller_saved_xmm_registers &= ~(1ull << (x64::xmm::XMM0 + static_cast<u8>(i)));
						}
						else {
							caller_saved_xmm_registers &= ~(1ull << default_return_registers[i].id);
						}
					}
				}

				// system calls don't count, we track this for ABI and stack allocation purposes
				if (context.caller_usage < n->inputs.get_size() - 3) {
					context.caller_usage = n->inputs.get_size() - 3;
				}

				// parameter passing is separate from eval due to register allocation
				u8 in_count = 0;
				i32 ins[64];
				reg parameter_registers[64];

				u64 vararg_cutoff;
				if(callee_signature.has_var_args) {
					vararg_cutoff = callee_signature.parameters.size();
				}
				else {
					vararg_cutoff = n->inputs.get_size() - 2;
				}

				u8 used_gpr_count = 0;
				u8 used_xmm_count = 0;

				for (u64 i = 3; i < n->inputs.get_size(); ++i) {
					handle<node> parameter_node = n->inputs[i];
					data_type param_dt = parameter_node->dt;

					bool use_xmm = param_dt == data_type::base::FLOAT;
					u8 reg = use_xmm ? used_xmm_count : used_gpr_count;

					if (is_systemv) {
						if (use_xmm) {
							used_xmm_count++;
						}
						else {
							used_gpr_count++;
							caller_saved_gp_registers &= ~(1u << used_gpr_count);
						}
					}
					else {
						// win64 will always expend a register
						used_xmm_count++;
						used_gpr_count++;
					}

					// first few parameters are passed as inputs to the CALL instruction.
					// the rest are written into the stack at specific places.
					sigma::ir::reg src = allocate_node_register(context, parameter_node);

					if (reg >= descriptor.gpr_count) {
						context.append_instruction(create_mr(
							context,
							use_xmm ? instruction::type::FP_MOV : instruction::type::MOV,
							parameter_node->dt,
							static_cast<u8>(x64::gpr::RSP),
							reg::invalid_id,
							memory_scale::x1, 
							reg * 8,
							src.id
						));
					}
					else {
						u8 phys_reg = use_xmm ? reg : descriptor.gpr_registers[reg].id;
						u8 dst = phys_reg;

						if(use_xmm) {
							dst += x64::register_class::FIRST_XMM;
						}
						else {
							dst += x64::register_class::FIRST_GPR;
						}

						context.hint_reg(src.id, dst);

						parameter_registers[in_count] = src;
						ins[in_count++] = dst;

						if (use_xmm) {
							caller_saved_xmm_registers &= ~(1ull << phys_reg);
						}
						else {
							caller_saved_gp_registers &= ~(1ull << phys_reg);
						}
					}
				}

				// perform last minute copies (this avoids keeping parameter registers alive for too long)
				for (u64 i = 0; i < in_count; ++i) {
					data_type dt = n->inputs[3 + i]->dt;
					bool use_xmm = dt == data_type::base::FLOAT;

					context.append_instruction(create_move(
						context, dt, static_cast<u8>(ins[i]), parameter_registers[i]
					));

					// in win64, float params past the vararg cutoff are duplicated in their
					// respective GPR slot
					if (use_xmm && i >= vararg_cutoff && i < descriptor.gpr_count) {
						u8 phys_reg = descriptor.gpr_registers[i].id;

						context.append_instruction(create_rr(
							context, instruction::type::MOV_F2I, I64_TYPE, phys_reg, static_cast<u8>(ins[i])
						));

						ins[in_count++] = x64::register_class::FIRST_GPR + phys_reg;
					}
				}

				// compute the target (unless it's a symbol) before the registers all need to be
				// forcibly shuffled
				handle<node> target = n->inputs[2];
				bool static_call = n != node::type::SYSTEM_CALL && target == node::type::SYMBOL;

				reg target_val = static_cast<u8>(x64::gpr::RSP);
				if (!static_call) {
					target_val = allocate_node_register(context, target);
				}

				if (node_type == node::type::SYSTEM_CALL) {
					NOT_IMPLEMENTED();
				}
				else {
					// the number of float parameters is written into AL
					if (callee_signature.has_var_args && is_systemv) {
						context.append_instruction(create_immediate(
							context, instruction::type::MOV, I8_TYPE, static_cast<u8>(x64::gpr::RAX), used_xmm_count
						));

						ins[in_count++] = x64::register_class::FIRST_GPR + x64::gpr::RAX;
						caller_saved_gp_registers &= ~(1ull << x64::gpr::RAX);
					}
				}

				// all these registers need to be spilled and reloaded if they're used across
				// the function call boundary... you might see why inlining could be nice to implement
				const u8 caller_saved_gpr_pop_count = utility::pop_count(caller_saved_gp_registers);
				const u8 caller_saved_xmm_pop_count = utility::pop_count(caller_saved_xmm_registers);
				u8 clobber_count = caller_saved_gpr_pop_count + caller_saved_xmm_pop_count;
				auto op = instruction::type::SYS_CALL;

				if (n == node::type::CALL) {
					op = instruction::type::CALL;
				}
				else if(n == node::type::TAIL_CALL) {
					op = instruction::type::JMP;
				}

				handle<instruction> call_inst = create_instruction<handle<symbol>>(
					context, op, PTR_TYPE, return_count, 1 + in_count, clobber_count
				);

				// mark the clobber list
				auto clobbers = call_inst->operands.begin() + call_inst->out_count + call_inst->in_count;
				for (u8 i = 0; i < 16; ++i) {
					if (caller_saved_gp_registers & (1u << i)) {
						*clobbers++ = x64::register_class::FIRST_GPR + i;
					}
				}

				for (u8 i = 0; i < 16; ++i) {
					if (caller_saved_xmm_registers & (1u << i)) {
						*clobbers++ = x64::register_class::FIRST_XMM + i;
					}
				}

				// handle the return value (either XMM0 or RAX)
				auto dst_ins = call_inst->operands.begin();

				for (u8 i = 0; i < 2; ++i) {
					if (return_nodes[i] != nullptr) {
						if (return_nodes[i]->dt == data_type::base::FLOAT) {
							*dst_ins++ = x64::register_class::FIRST_XMM + i;
						}
						else {
							*dst_ins++ = x64::register_class::FIRST_GPR + i;
						}
					}
				}

				// write inputs
				if (static_call) {
					call_inst->flags |= instruction::GLOBAL;
					call_inst->memory.index = call_inst->out_count;
					call_inst->get<handle<symbol>>() = target->get<handle<symbol>>();
				}

				*dst_ins++ = target_val.id;
				memcpy(dst_ins, ins, in_count * sizeof(i32));
				context.append_instruction(call_inst);

				// copy out return
				for (u8 i = 0; i < 2; ++i) {
					if (return_nodes[i] != nullptr) {
						ASSERT(return_registers[i].is_valid(), "invalid return register detected");
						data_type dt = return_nodes[i]->dt;

						if (dt == data_type::base::FLOAT) {
							context.hint_reg(return_registers[i].id, x64::register_class::FIRST_GPR + i);

							context.append_instruction(create_move(
								context, dt, return_registers[i], x64::register_class::FIRST_XMM + i
							));
						}
						else {
							context.hint_reg(return_registers[i].id, default_return_registers[i]);

							context.append_instruction(create_move(
								context, dt, return_registers[i], default_return_registers[i]
							));
						}
					}
				}

				break;
			}

			case node::type::CMP_EQ:
			case node::type::CMP_NE:
			case node::type::CMP_SLT:
			case node::type::CMP_SLE:
			case node::type::CMP_ULT:
			case node::type::CMP_ULE:
			case node::type::CMP_FLT:
			case node::type::CMP_FLE: {
				x64::conditional cc = select_instruction_cmp(context, n);
				instruction::type type = static_cast<instruction::type::underlying>(static_cast<i32>(instruction::type::SETO) + static_cast<instruction::type::underlying>(cc));
				context.append_instruction(create_r(context, type, I8_TYPE, destination));
				break;
			}

			case node::type::INTEGER_CONSTANT: {
				u64 value = n->get<integer>().value;

				// mask off bits
				u64 bits_in_type;
				if(n->dt == data_type::base::POINTER) {
					bits_in_type = 64;
				}
				else {
					bits_in_type = n->dt.get_bit_width();
				}

				if (bits_in_type < 64) {
					value &= (1ull << bits_in_type) - 1;
				}

				if (value == 0) {
					context.append_instruction(create_zero(
						context, n->dt, destination
					));
				}
				else if ((value >> 32ull) == std::numeric_limits<u32>::max()) {
					// mov but zero ext
					context.append_instruction(create_immediate(
						context, instruction::type::MOV, I32_TYPE, destination, static_cast<i32>(value)
					));
				}
				else if (bits_in_type <= 32 || (value >> 31ull) == 0) {
					context.append_instruction(create_immediate(
						context, instruction::type::MOV, n->dt, destination, static_cast<i32>(value)
					));
				}
				else {
					// movabs reg, imm64
					context.append_instruction(create_abs(
						context, instruction::type::MOVABS, n->dt, destination, value
					));
				}

				break;
			}

			case node::type::AND:
			case node::type::OR:
			case node::type::XOR:
			case node::type::ADD:
			case node::type::SUB: {
				static instruction::type operations[] = {
					instruction::type::AND,
					instruction::type::OR,
					instruction::type::XOR,
					instruction::type::ADD,
					instruction::type::SUB,
				};

				const instruction::type operation = operations[node_type - node::type::AND];
				const reg left = allocate_node_register(context, n->inputs[1]);
				i32 immediate;

				context.hint_reg(destination.id, left);

				if (n->inputs[2] == node::type::LOAD && n->inputs[2]->is_on_last_use(context)) {
					n->inputs[2]->use_node(context);

					context.append_instruction(create_move(
						context, n->dt, destination, left
					));

					handle<instruction> inst = select_array_access_instruction(
						context, n->inputs[2]->inputs[2], destination, -1, destination.id
					);

					inst->set_type(operation);
					inst->data_type = legalize_data_type(n->dt);

					context.append_instruction(inst);
				}
				else if (try_for_imm32(n->inputs[2], n->dt.get_bit_width(), immediate)) {
					n->inputs[2]->use_node(context);

					if (node_type == node::type::ADD) {
						context.append_instruction(create_rm(
							context, instruction::type::LEA, I64_TYPE, destination, left, -1, memory_scale::x1, immediate
						));
					}
					else {
						context.append_instruction(create_move(
							context, n->dt, destination, left
						));

						context.append_instruction(create_rri(
							context, operation, n->dt, destination, destination, immediate
						));
					}
				}
				else {
					reg right = allocate_node_register(context, n->inputs[2]);

					context.append_instruction(create_move(
						context, n->dt, destination, left
					));

					context.append_instruction(create_rrr(
						context, operation, n->dt, destination, destination, right
					));
				}

				break;
			}

			case node::type::MUL: {
				reg left = allocate_node_register(context, n->inputs[1]);
				context.hint_reg(destination.id, left);

				data_type dt = n->dt;
				ASSERT(dt == data_type::base::INTEGER, "invalid type for a MUL op");

				if(dt.get_bit_width() < 16) {
					dt.set_bit_width(16);
				}

				i32 x;
				if(try_for_imm32(n->inputs[2], dt.get_bit_width(), x)) {
					const handle<virtual_value> v = context.lookup_virtual_value(n->inputs[2]);
					if(v) {
						v->use_count -= 1;
					}

					context.append_instruction(create_move(context, dt, destination, left));
					context.append_instruction(create_rri(
						context, instruction::type::IMUL, dt, destination, destination, x)
					);
				}
				else {
					reg right = allocate_node_register(context, n->inputs[2]);

					context.append_instruction(create_move(context, dt, destination, left));
					context.append_instruction(create_rrr(
						context, instruction::type::IMUL, dt, destination, destination, right)
					);
				}

				break;
			}

			case node::type::NOT:
			case node::type::NEG: {
				if(!n->dt.is_floating_point()) {
					reg src = allocate_node_register(context, n->inputs[1]);
					bool is_not = n->get_type() == node::type::NOT;
					instruction::type type = is_not ? instruction::type::NOT : instruction::type::NEG;

					context.append_instruction(create_move(context, n->dt, destination, src));
					context.append_instruction(create_rr(context, type, n->dt, destination, destination));
				}
				else {
					NOT_IMPLEMENTED();
				}

				break;
			}

			case node::type::EXIT: {
				ASSERT(n->inputs.get_size() <= 5, "at most 2 return values :(");
				static reg default_return_registers[2] = {
					static_cast<u8>(x64::gpr::RAX), static_cast<u8>(x64::gpr::RDX)
				};

				const u64 return_count = n->inputs.get_size() - 3;

				for (u64 i = 0; i < return_count; ++i) {
					reg source = allocate_node_register(context, n->inputs[3 + i]);
					auto dt = n->inputs[3 + i]->dt;

					// copy to the return register
					if (dt == data_type::base::FLOAT) {
						NOT_IMPLEMENTED();
					}
					else {
						context.hint_reg(source.id, default_return_registers[i]);
						context.append_instruction(create_move(
							context, dt, default_return_registers[i], source)
						);
					}
				}

				// we don't really need a fence if we're about to exit but we do need to mark that
				// it's the epilogue to tell the register allocator where callee registers need
				// to get restored
				handle<instruction> inst = create_instruction(
					context, instruction::type::EPILOGUE, VOID_TYPE, 0, 0, 0);

				inst->flags |= instruction::RET;

				context.append_instruction(inst);


				break;
			}

			case node::type::LOCAL:
			case node::type::VARIADIC_START:
			case node::type::MEMBER_ACCESS:
			case node::type::ARRAY_ACCESS: {
				context.append_instruction(select_memory_access_instruction(
					context, n, destination, -1, -1)
				);
				break;
			}

			case node::type::STORE: {
				if (destination.is_valid()) {
					n->inputs[2]->use_node(context);
					n->inputs[3]->use_node(context);
					break;
				}

				const data_type store_data_type = n->inputs[3]->dt;
				const handle<node> address = n->inputs[2];
				handle<node> source_node = n->inputs[3];
				i32 immediate_value;

				// check if we can couple the load and the store operations
				i32 store_op = can_folded_store(context,n->inputs[1], address, n->inputs[3]);

				if (store_op >= 0) {
					source_node->use_node(context);
					address->use_node(context);
					source_node->inputs[1]->use_node(context);
					source_node->inputs[1]->inputs[1]->use_node(context);

					source_node = source_node->inputs[2];
				}
				else {
					if (store_data_type == data_type::base::FLOAT) {
						store_op = instruction::type::FP_MOV;
					}
					else {
						store_op = instruction::type::MOV;
					}
				}

				if(try_for_imm32(source_node, source_node->dt.is_pointer() ? 64 : source_node->dt.get_bit_width(), immediate_value)) {
					source_node->use_node(context);

					const handle<instruction> store_inst = select_array_access_instruction(
						context, address, destination, store_op, -1
					);

					store_inst->in_count = store_inst->in_count - 1;
					store_inst->data_type = legalize_data_type(store_data_type);
					store_inst->flags |= instruction::IMMEDIATE;
					store_inst->set_property(context.function->allocator.allocate(sizeof(immediate)));
					store_inst->get<immediate>().value = immediate_value;

					ASSERT(
						store_inst->flags & (instruction::MEM | instruction::GLOBAL),
						"invalid store instruction flags"
					);

					context.append_instruction(store_inst);
				}
				else {
					const reg source = allocate_node_register(context, source_node);
					const handle<instruction> store_inst = select_array_access_instruction(
						context, address, destination, store_op, source.id
					);

					store_inst->data_type = legalize_data_type(store_data_type);

					ASSERT(
						store_inst->flags & (instruction::MEM | instruction::GLOBAL),
						"invalid store instruction flags"
					);

					context.append_instruction(store_inst);
				}

				break;
			}

			case node::type::BRANCH: {
				auto& br = n->get<branch>();

				std::vector<u64> successors(br.successors.size());
			  // bool has_default = false;

				// fill successors
				for (handle<user> u = n->use; u; u = u->next_user) {
					if (u->target == node::type::PROJECTION) {
						u64 index = u->target->get<projection>().index;
						handle<node> successor_node = u->target->get_next_block();

						// if (index == 0) {
						// 	has_default = !successor_node->is_unreachable();
						// }

						successors[index] = context.graph.blocks.at(successor_node).id;
					}
				}

				data_type dt = n->inputs[1]->dt;

				context.append_instruction(create_instruction(
					context, instruction::type::TERMINATOR, VOID_TYPE, 0, 0, 0)
				);

				if (br.successors.size() == 1) {
					PANIC("degenerate branch");
				}
				else if (br.successors.size() == 2) {
					u64 t = successors[0];
					u64 f = successors[1];

					auto cc = x64::conditional::E;
					if (br.keys[0] == 0) {
						cc = select_instruction_cmp(context, n->inputs[1]);
					}
					else {
						reg key = allocate_node_register(context, n->inputs[1]);
						context.append_instruction(create_ri(
							context, instruction::type::CMP, dt, key, static_cast<i32>(br.keys[0]))
						);

						cc = x64::conditional::NE;
					}

					// if flipping avoids a jmp, do that
					if (context.fallthrough == t) {
						context.append_instruction(
							create_jcc(context, static_cast<i32>(f), static_cast<x64::conditional>(cc ^ 1))
						);
					}
					else {
						context.append_instruction(create_jcc(context, static_cast<i32>(t), cc));

						if (context.fallthrough != f) {
							context.append_instruction(create_jump(context, f));
						}
					}
				}
				else {
					NOT_IMPLEMENTED();
				}

				break;
			}

			case node::type::SYMBOL: {
				auto s = n->get<handle<symbol>>();

				context.append_instruction(create_op_global(
					context, instruction::type::LEA, n->dt, destination, s
				));

				break;
			}
			case node::type::LOAD:
			case node::type::ATOMIC_LOAD: {
				instruction::type mov_op;
				if(n->dt == data_type::base::FLOAT) {
					mov_op = instruction::type::FP_MOV;
				}
				else {
					mov_op = instruction::type::MOV;
				}

				handle<node> address = n->inputs[2];

				handle<instruction> load_inst = select_array_access_instruction(
					context, address, destination, -1, -1
				);

				load_inst->set_type(mov_op);
				load_inst->data_type = legalize_data_type(n->dt);

				if(n == node::type::ATOMIC_LOAD) {
					load_inst->flags |= instruction::LOCK;
				}

				context.append_instruction(load_inst);
				break;
			}

			case node::type::PROJECTION: {
				if (n->inputs[0] == node::type::ENTRY) {
					const i32 index = static_cast<i32>(n->get<projection>().index - 3);
					i32 param_gpr_count = context.target.get_abi() == abi::WIN_64 ? 4 : 6;

					// use stack space past the n registers we're given
					if (index >= param_gpr_count) {
						instruction::type i;
						if(n->dt == data_type::base::FLOAT) {
							i = instruction::type::FP_MOV;
						}
						else {
							i = instruction::type::MOV;
						}

						context.append_instruction(create_rm(
							context, 
							i,
							n->dt,
							destination,
							static_cast<u8>(x64::gpr::RBP),
							reg::invalid_id, 
							memory_scale::x1, 
							16 + index * 8
						));
					}
				}

				break;
			}
			default: {
				NOT_IMPLEMENTED();
			}
		}
	}

	auto x64_architecture::select_memory_access_instruction(codegen_context& context, handle<node> n, reg dst, i32 store_op, i32 src) -> handle<instruction> {
		const bool has_second_in = store_op < 0 && src >= 0;
		i32 offset = 0;

		if (n == node::type::SYMBOL) {
			NOT_IMPLEMENTED();
		}
		else if (n == node::type::VARIADIC_START) {
			NOT_IMPLEMENTED();
		}
		else if (n == node::type::MEMBER_ACCESS) {
			offset = static_cast<i32>(n->get<member>().offset);

			n->use_node(context);
			n = n->inputs[1];
		}

		auto scale = memory_scale::x1;
		i32 index = -1;

		if(n == node::type::ARRAY_ACCESS) {
			const handle<node> base = n->inputs[1];
			i64 stride = n->get<array>().stride;

			n->use_node(context);
			n = n->inputs[2];

			i32 x = 0;
			if (n->get_type() == node::type::SHL && try_for_imm32(n->inputs[2], 64, x)) {
				n->use_node(context);
				n->inputs[2]->use_node(context);

				n = n->inputs[1];
				stride *= 1ull << x;
			}

			index = allocate_node_register(context, n).id;

			if(stride == 1) { 
				// no scaling required
			}
			else if (utility::is_power_of_two(stride)) {
				scale = static_cast<memory_scale>(utility::ffs(static_cast<i32>(stride)) - 1);

				if(static_cast<i32>(scale) > 3) {
					if(!dst.is_valid()) {
						ASSERT(store_op >= 0, "panic");
						dst = allocate_virtual_register(context, nullptr, I64_TYPE);
					}

					// we can't fit this into an LEA, might as well just do a shift
					context.append_instruction(create_rri(
						context, instruction::type::SHL, I64_TYPE, dst, static_cast<reg::id_type>(index), static_cast<i32>(scale)
					));

					index = dst.id;
					scale = static_cast<memory_scale>(0);
				}
			}
			else {
				if(!dst.is_valid()) {
					ASSERT(store_op >= 0, "panic");
					dst = allocate_virtual_register(context, nullptr, I64_TYPE);
				}

				context.append_instruction(create_rri(
					context, instruction::type::IMUL, I64_TYPE, dst, static_cast<reg::id_type>(index), static_cast<i32>(stride)
				));

				index = dst.id;
			}

			n = base;
		}

		reg base;

		if (n == node::type::LOCAL) {
			n->use_node(context);

			offset += context.get_stack_slot(n);
			base = static_cast<u8>(x64::gpr::RBP);
		}
		else {
			base = allocate_node_register(context, n);
		}

		// compute the base
		if(store_op < 0) {
			if(has_second_in) {
				return create_rrm(context, instruction::type::LEA, n->dt, dst, static_cast<u8>(src), base, index, scale, offset);
			}

			return create_rm(context, instruction::type::LEA, n->dt, dst, base, index, scale, offset);
		}

		return create_mr(
			context,
			static_cast<instruction::type::underlying>(store_op),
			n->dt,
			base, 
			static_cast<reg::id_type>(index),
			scale, 
			offset,
			src
		);
	}

	auto x64_architecture::select_array_access_instruction(codegen_context& context, handle<node> n, reg dst, i32 store_op, i32 src) -> handle<instruction> {
		// compute base
		if (n == node::type::ARRAY_ACCESS) {
			if(context.virtual_values.at(n->global_value_index).use_count > 2 || context.virtual_values.at(n->global_value_index).virtual_register.is_valid()) {
				const reg base = allocate_node_register(context, n);

				if (store_op < 0) {
					if (src >= 0) {
						return create_rrm(context, instruction::type::LEA, PTR_TYPE, dst, static_cast<u8>(src), base, -1, memory_scale::x1, 0);
					}

					return create_rm(context, instruction::type::LEA, PTR_TYPE, dst, base, -1, memory_scale::x1, 0);
				}

				return create_mr(context, static_cast<instruction::type::underlying>(store_op), PTR_TYPE, base, reg::invalid_id, memory_scale::x1, 0, src);
			}
		}

		return select_memory_access_instruction(context, n, dst, store_op, src);
	}

	auto x64_architecture::select_instruction_cmp(codegen_context& context, handle<node> n) -> x64::conditional {
		bool invert = false;
		if(n->get_type() == node::type::CMP_EQ && n->dt == data_type::base::INTEGER && n->dt.get_bit_width() == 1 && n->inputs[2]->get_type() == node::type::INTEGER_CONSTANT) {
			const auto& b = n->inputs[2]->get<integer>();
			if (b.value == 0) {
				invert = true;
				n = n->inputs[1];
			}
		}

		if(n->get_type() >= node::type::CMP_EQ && n->get_type() <= node::type::CMP_FLE) {
			const data_type cmp_dt = n->get<compare_op>().cmp_dt;
			auto cc = static_cast<x64::conditional>(-1);

			n->use_node(context);

			if(cmp_dt.is_floating_point()) {
				const reg lhs = allocate_node_register(context, n->inputs[1]);
				const reg rhs = allocate_node_register(context, n->inputs[2]);

				context.append_instruction(create_rrd(context, instruction::type::FP_UCOMI, cmp_dt, lhs, rhs));

				switch(n->get_type()) {
					case node::type::CMP_EQ:  cc = x64::conditional::E;  break;
					case node::type::CMP_NE:  cc = x64::conditional::NE; break;
					case node::type::CMP_FLT: cc = x64::conditional::B;  break;
					case node::type::CMP_FLE: cc = x64::conditional::BE; break;
					default: NOT_IMPLEMENTED();
				}
			}
			else {
				invert = false;
				i32 x;
				const reg lhs = allocate_node_register(context, n->inputs[1]);
				const i32 bits = cmp_dt.is_pointer() ? 64 : cmp_dt.get_bit_width();

				if(try_for_imm32(n->inputs[2], bits, x)) {
					n->inputs[2]->use_node(context);

					if(x == 0 && (n->get_type() == node::type::CMP_EQ || n->get_type() == node::type::CMP_NE)) {
						context.append_instruction(create_rrd(context, instruction::type::TEST, cmp_dt, lhs, lhs));
					}
					else {
						context.append_instruction(create_ri(context, instruction::type::CMP, cmp_dt, lhs, x));
					}
				}
				else if(
					n->inputs[2]->get_type() == node::type::LOAD &&
					n->inputs[2]->is_on_last_use(context)
				) {
					n->inputs[2]->use_node(context);

					const handle<instruction> inst = select_array_access_instruction(
						context, n->inputs[2]->inputs[2], lhs, -1, lhs.id
					);

					inst->set_type(instruction::type::CMP);
					inst->data_type = legalize_data_type(n->dt);
					context.append_instruction(inst);
				}
				else {
					const reg rhs = allocate_node_register(context, n->inputs[2]);
					context.append_instruction(create_rrd(context, instruction::type::CMP, cmp_dt, lhs, rhs));
				}

				switch(n->get_type()) {
					case node::type::CMP_EQ: cc = x64::conditional::E; break;
					case node::type::CMP_NE: cc = x64::conditional::NE; break;
					case node::type::CMP_SLT: cc = invert ? x64::conditional::G :  x64::conditional::L;  break;
					case node::type::CMP_SLE: cc = invert ? x64::conditional::GE : x64::conditional::LE; break;
					case node::type::CMP_ULT: cc = invert ? x64::conditional::A :  x64::conditional::B;  break;
					case node::type::CMP_ULE: cc = invert ? x64::conditional::NB : x64::conditional::BE; break;
					default: NOT_IMPLEMENTED();
				}
			}

			return static_cast<x64::conditional>(cc ^ invert);
		}
		else {
			const reg src = allocate_node_register(context, n);
			const data_type dt = n->dt;

			if (dt == data_type::base::FLOAT) {
				NOT_IMPLEMENTED();
			}
			else {
				context.append_instruction(create_rrd(context, instruction::type::TEST, dt, src, src));
			}

			return static_cast<x64::conditional>(x64::conditional::NE ^ invert);
		}
	}

	void x64_architecture::dfs_schedule(codegen_context& context, handle<basic_block> bb, handle<node> n, bool is_end) {
		const auto it = context.schedule.find(n);
		if (it == context.schedule.end() || it->second != bb || !context.work.visit(n)) {
			return;
		}

		// if we're a branch, push our PHI nodes
		if (is_end) {
			const bool is_fallthrough = n != node::type::BRANCH;

			for (handle<user> user = n->use; user; user = user->next_user) {
				if (!user->target->is_control()) {
					continue;
				}

				handle<node> destination;
				if(is_fallthrough) {
					destination = user->target;
				}
				else {
					destination = user->target->get_next_block();
				}

				// find predecessor index and do that edge
				ptr_diff phi_index = -1;
				for (u64 j = 0; j < destination->inputs.get_size(); ++j) {
					if (context.schedule.at(destination->inputs[j]) == bb) {
						phi_index = static_cast<ptr_diff>(j);
						break;
					}
				}

				if (phi_index < 0) {
					continue;
				}

				// schedule memory phis
				for (auto use = destination->use; use; use = use->next_user) {
					const handle<node> phi = use->target;

					if (phi == node::type::PHI && phi->dt == data_type::base::MEMORY) {
						dfs_schedule_phi(context, bb, phi, phi_index);
					}
				}

				// schedule data phis, we schedule these afterwards because it's "generally" better
				for (auto use = destination->use; use; use = use->next_user) {
					const handle<node> phi = use->target;

					if (phi == node::type::PHI && phi->dt != data_type::base::MEMORY) {
						dfs_schedule_phi(context, bb, phi, phi_index);
					}
				}
			}
		}

		// push inputs
		for (u64 i = n->inputs.get_size(); i-- > 0;) {
			if (const handle<node> input = n->inputs[i]) {
				dfs_schedule(context, bb, input, false);
			}
		}

		// before the terminator we should eval leftovers that GCM linked here
		if (is_end) {
			for (const auto& entry : bb->items) {
				dfs_schedule(context, bb, entry, false);
			}
		}

		context.work.items.push_back(n);

		if (n->is_mem_out_op() && n != node::type::PHI && n != node::type::PROJECTION) {
			// memory effects have anti-dependencies, the previous loads
			// must finish before the next memory effect is applied.
			for (handle<user> use = n->inputs[1]->use; use; use = use->next_user) {
				if (use->slot == 1 && use->target != n) {
					dfs_schedule(context, bb, use->target, false);
				}
			}
		}

		// push outputs (projections, if they apply)
		if (n->dt == data_type::base::TUPLE && n != node::type::BRANCH) {
			for (auto use = n->use; use; use = use->next_user) {
				const handle<node> projection = use->target;

				if (projection == node::type::PROJECTION) {
					dfs_schedule(context, bb, projection, false);
				}
			}
		}
	}

	void x64_architecture::dfs_schedule_phi(codegen_context& context, handle<basic_block> bb, handle<node> phi, ptr_diff phi_index) {
		const handle<node> value = phi->inputs[1 + phi_index];

		// reserve phi space
		if (phi->dt != data_type::base::MEMORY) {
			context.phi_values.emplace_back(phi_value{ 
				.phi = phi, 
				.target = value
			});
		}

		dfs_schedule(context, bb, value, false);
	}

	auto x64_architecture::classify_register_class(const data_type& data_type) -> u8 {
		return data_type == data_type::base::FLOAT ? x64::register_class::XMM : x64::register_class::GPR;
	}

	auto x64_architecture::allocate_virtual_register(codegen_context& context, handle<node> target, const data_type& data_type) -> reg {
		const u64 index = context.intervals.size();

		classified_reg reg;
		reg.cl = classify_register_class(data_type);

		// create a new live interval with an uninitialized register
		live_interval it{
			.assigned = reg::invalid_id,
			.reg = reg,
			.target = target,
			.data_type = legalize_data_type(data_type),
			.ranges = { utility::range<u64>::max() }
		};

		context.intervals.emplace_back(it);
		ASSERT(index < std::numeric_limits<u8>::max(), "invalid virtual register");
		return { static_cast<reg::id_type>(index)};
	}

	auto x64_architecture::can_folded_store(codegen_context& context, handle<node> memory, handle<node> address, handle<node> source) -> i32 {
		switch (source->get_type()) {
			default: {
				return -1;
			}

			case node::type::AND:
			case node::type::OR:
			case node::type::XOR:
			case node::type::ADD:
			case node::type::SUB: {
				if (
					source->inputs[1] == node::type::LOAD &&
					source->inputs[1]->inputs[1] == memory &&
					source->inputs[1]->inputs[2] == address &&
					source->is_on_last_use(context) &&
					source->inputs[1]->is_on_last_use(context)
				) {
					static instruction::type operations[] = {
						instruction::type::AND,
						instruction::type::OR,
						instruction::type::XOR,
						instruction::type::ADD,
						instruction::type::SUB,
					};

					return operations[source->get_type() - node::type::AND];
				}

				return -1;
			}
		}
	}

	auto x64_architecture::try_for_imm32(handle<node> n, i32 bits, i32& out) -> bool {
		if (n != node::type::INTEGER_CONSTANT) {
			return false;
		}

		const integer& i = n->get<integer>();

		if (bits > 32) {
			bool sign = (i.value >> 31ull) & 1;
			uint64_t top = i.value >> 32ull;

			// if the sign matches the rest of the top bits, we can sign extend just fine
			if (top != (sign ? 0xFFFFFFFF : 0)) {
				return false;
			}
		}

		out = static_cast<i32>(i.value);
		return true;
	}

	auto x64_architecture::allocate_node_register(codegen_context& context, handle<node> target) -> reg {
		// attempt to lookup an existing value
		const handle<virtual_value> value = context.lookup_virtual_value(target);

		// ASSERT(n->global_value_index != 5, "x");
		// no value was found, allocate a new virtual register
		if (value == nullptr) {
			const reg tmp = allocate_virtual_register(context, target, target->dt);
			select_instruction(context, target, tmp);
			return tmp;
		}

		value->use_count--;

		// if we have a virtual register, return its ID
		if (value->virtual_register.is_valid()) {
			return value->virtual_register;
		}

		// if the node should rematerialize we allocate a new virtual register
		if (target->should_rematerialize()) {
			const reg tmp = allocate_virtual_register(context, target, target->dt);
			select_instruction(context, target, tmp);
			return tmp;
		}

		// fallback to just allocating a new virtual register
		const reg tmp = allocate_virtual_register(context, target, target->dt);
		value->virtual_register = tmp;
		return tmp;
	}

	auto x64_architecture::create_label(codegen_context& context, handle<node> target) -> handle<instruction> {
		const handle<instruction> inst = context.create_instruction<handle<node>>(0);
		inst->get<handle<node>>() = target;
		inst->set_type(instruction::type::LABEL);
		inst->flags = instruction::NODE;
		return inst;
	}

	auto x64_architecture::create_jump(codegen_context& context, u64 target) -> handle<instruction> {
		const handle<instruction> inst = create_instruction<label>(context, instruction::type::JMP, VOID_TYPE, 0, 0, 0);
		inst->flags = instruction::NODE;
		inst->get<label>().value = target;
		return inst;
	}

	auto x64_architecture::create_jcc(codegen_context& context, int target, x64::conditional cc) -> handle<instruction>{
		const handle<instruction> inst = create_instruction<label>(context, static_cast<instruction::type::underlying>(instruction::type::JO + cc), VOID_TYPE, 0, 0, 0);
		inst->flags = instruction::NODE;
		inst->get<label>().value = target;
		return inst;
	}

	auto x64_architecture::create_move(codegen_context& context, const data_type& data_type, reg destination, reg source) -> handle<instruction> {
		const i32 machine_data_type = legalize_data_type(data_type);
		const handle<instruction> inst = context.create_instruction(2);

		inst->set_type(machine_data_type >= x64::SSE_SS ? instruction::type::FP_MOV : instruction::type::MOV);
		inst->data_type = machine_data_type;

		inst->out_count = 1;
		inst->in_count = 1;
		inst->operands[0] = destination.id;
		inst->operands[1] = source.id;
		return inst;
	}

	auto x64_architecture::create_r(codegen_context& context, instruction::type type, const data_type& data_type, reg dst) -> handle<instruction> {
		const handle<instruction> inst = create_instruction(context, type, data_type, 1, 0, 0);
		inst->operands[0] = dst.id;
		return inst;
	}

	auto x64_architecture::create_mr(codegen_context& context, instruction::type type, const data_type& data_type, reg base, reg index, memory_scale scale, i32 disp, i32 source) -> handle<instruction> {
		const handle<instruction> inst = create_instruction(context, type, data_type, 0, index.is_valid() ? 3 : 2, 0);

		inst->flags = instruction::MEM | (index.is_valid() ? instruction::INDEXED : instruction::NONE);
		inst->memory.index = 0;

		inst->operands[0] = base.id;

		if (index.is_valid()) {
			inst->operands[1] = index.id;
			inst->operands[2] = source;
		}
		else {
			inst->operands[1] = source;
		}

		inst->memory.displacement = disp;
		inst->memory.scale = scale;
		return inst;
	}

	auto x64_architecture::create_rm(codegen_context& context, instruction::type type, const data_type& data_type, reg destination, reg base, i32 index, memory_scale scale, i32 disp) -> handle<instruction> {
		const handle<instruction> inst = create_instruction(context, type, data_type, 1, index >= 0 ? 2 : 1, 0);

		inst->flags = instruction::MEM | (index >= 0 ? instruction::INDEXED : instruction::NONE);
		inst->memory.index = 1;
		inst->operands[0] = destination.id;
		inst->operands[1] = base.id;

		if (index >= 0) {
			inst->operands[2] = index;
		}

		inst->memory.displacement = disp;
		inst->memory.scale = scale;
		return inst;
	}

	auto x64_architecture::create_rr(codegen_context& context, instruction::type type, const data_type& data_type, reg destination, reg source) -> handle<instruction> {
		const handle<instruction> inst = create_instruction(context, type, data_type, 1, 1, 0);
		inst->operands[0] = destination.id;
		inst->operands[1] = source.id;
		return inst;
	}

	auto x64_architecture::create_rrd(codegen_context& context, instruction::type type, const data_type& data_type, reg destination, reg source) -> handle<instruction> {
		const handle<instruction> inst = create_instruction(context, type, data_type, 0, 2, 0);
		inst->operands[0] = destination.id;
		inst->operands[1] = source.id;
		return inst;
	}

	auto x64_architecture::create_immediate(codegen_context& context, instruction::type type, const data_type& data_type, reg destination, i32 value) -> handle<instruction> {
		const handle<instruction> inst = create_instruction<immediate>(context, type, data_type, 1, 0, 0);
		inst->get<immediate>().value = value;
		inst->flags = instruction::IMMEDIATE;
		inst->operands[0]= destination.id;
		return inst;
	}

	auto x64_architecture::create_zero(codegen_context& context, const data_type& data_type, reg destination) -> handle<instruction> {
		const handle<instruction> inst = create_instruction(context, instruction::type::ZERO, data_type, 1, 0, 0);
		inst->operands[0] = destination.id;
		return inst;
	}

	auto x64_architecture::create_abs(codegen_context& context, instruction::type type, const data_type& data_type, reg destination, u64 immediate) -> handle<instruction> {
		const handle<instruction> inst = create_instruction<absolute>(context, type, data_type, 1, 0, 0);
		inst->get<absolute>().value = immediate;
		inst->flags = instruction::ABSOLUTE;
		inst->operands[0] = destination.id;
		return inst;
	}

	auto x64_architecture::create_ri(codegen_context& context, instruction::type type, const data_type& data_type, reg src, i32 imm) -> handle<instruction> {
		const handle<instruction> inst = create_instruction<absolute>(context, type, data_type, 0, 1, 0);
		inst->flags = instruction::IMMEDIATE;
		inst->operands[0] = src.id;
		inst->get<immediate>().value = imm;
		return inst;
	}

	auto x64_architecture::create_rri(codegen_context& context, instruction::type type, const data_type& data_type, reg destination, reg source, i32 immediate_value) -> handle<instruction> {
		const handle<instruction> inst = create_instruction<immediate>(context, type, data_type, 1, 1, 0);

		inst->get<immediate>().value = immediate_value;
		inst->flags = instruction::IMMEDIATE;
		inst->operands[0] = destination.id;
		inst->operands[1] = source.id;

		return inst;
	}

	auto x64_architecture::create_rrr(codegen_context& context, instruction::type type, const data_type& data_type, reg destination, reg left, reg right) -> handle<instruction> {
		const handle<instruction> inst = create_instruction<immediate>(context, type, data_type, 1, 2, 0);

		inst->operands[0] = destination.id;
		inst->operands[1] = left.id;
		inst->operands[2] = right.id;

		return inst;
	}

	auto x64_architecture::create_rrm(codegen_context& context, instruction::type type, const data_type& data_type, reg destination, reg source, reg base, i32 index, memory_scale scale, i32 disp) -> handle<instruction> {
		const handle<instruction> inst = create_instruction(context, type, data_type, 1, index >= 0 ? 3 : 2, 0);

		inst->flags = instruction::MEM | (index >= 0 ? instruction::INDEXED : instruction::NONE);
		inst->memory.scale = scale;
		inst->memory.displacement = disp;
		inst->memory.index = 2;

		inst->operands[0] = destination.id;
		inst->operands[1] = source.id;
		inst->operands[2] = base.id;

		if (index >= 0) {
			inst->operands[4] = index;
		}

		return inst;
	}

	auto x64_architecture::create_op_global(codegen_context& context,instruction::type type, const data_type& data_type, reg dst, handle<symbol> s) -> handle<instruction> {
		const handle<instruction> inst = create_instruction<handle<symbol>>(context, type, data_type, 1, 1, 0);

		inst->flags = instruction::GLOBAL;
		inst->memory.index = 1;
		inst->operands[0] = dst.id;
		inst->operands[1] = static_cast<u8>(x64::gpr::RSP);
		inst->get<handle<symbol>>() = s;
		inst->memory.displacement = 0;

		return inst;
	}

	auto x64_architecture::legalize_data_type(const data_type& data_type) -> i32 {
		if (data_type == data_type::base::FLOAT) {
			NOT_IMPLEMENTED();
		}

		u64 mask;
		return legalize_integer_data_type(&mask, data_type);
	}

	auto x64_architecture::legalize_integer_data_type(u64* out_mask, const data_type& data_type) -> x64::data_type {
		ASSERT(data_type == data_type::base::INTEGER || data_type == data_type::base::POINTER, "invalid type for integer legalization");

		if (data_type == data_type::base::POINTER) {
			*out_mask = 0;
			return x64::QWORD;
		}

		x64::data_type t = x64::NONE;
		const u8 bit_width = data_type.get_bit_width();
		i32 bits = 0;

		if (bit_width <= 8) {
			bits = 8;
			t = x64::BYTE;
		}
		else if (bit_width <= 16) {
			bits = 16;
			t = x64::WORD;
		}
		else if (bit_width <= 32) {
			bits = 32;
			t = x64::DWORD;
		}
		else if (bit_width <= 64) {
			bits = 64;
			t = x64::QWORD;
		}

		ASSERT(bits != 0, "TODO: large int support");
		const u64 mask = ~UINT64_C(0) >> (64 - bit_width);
		*out_mask = (bit_width == bits) ? 0 : mask;
		return t;
	}
} // namespace sigma::ir
