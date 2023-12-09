#include "x64_disassembler.h"
#include "intermediate_representation/target/arch/x64/x64.h"

namespace ir {
	auto x64_disassembler::disassemble(
		const utility::byte_buffer& bytecode, const codegen_context& context
	) -> utility::string {
		utility::string assembly;
		assembly.append("{}:\n", context.function->symbol.name);

		// disassemble the prologue
		disassemble_block(
			bytecode, 
			context,
			nullptr,
			std::numeric_limits<u64>::max(),
			{ 0, context.prologue_length },
			assembly
		);

		// disassemble the function body
		handle<symbol_patch> patch = context.first_patch;

		for(u64 i = 0; i < context.basic_block_order.size(); ++i) {
			const u64 block_index = context.basic_block_order[i];
			handle<node> basic_block = context.work_list->items[block_index];

			const u64 start = context.labels[block_index] & ~0x80000000;
			u64 end   = bytecode.get_size();

			if(i + 1 < context.basic_block_order.size()) {
				end = context.labels[context.basic_block_order[i + 1]] & ~0x80000000;
			}

			patch = disassemble_block(
				bytecode,
				context,
				patch,
				block_index,
				{ start, end },
				assembly
			);
		}

		return assembly;
	}

	handle<symbol_patch> x64_disassembler::disassemble_block(
		const utility::byte_buffer& bytecode,
		const codegen_context& context,
		handle<symbol_patch> patch,
		u64 basic_block,
		utility::range<u64> range,
		utility::string& assembly
	) {
		if(basic_block != std::numeric_limits<u64>::max()) {
			assembly.append(".bb{}:\n", basic_block);
		}

		while(range.start < range.end) {
			x64::x64_instruction inst;

			if(!disassemble_instruction(bytecode.get_slice(range.start, range.end - range.start), inst)) {
				range.start++;
				assembly.append("  ERROR\n");
				continue;
			}

			assembly.append("  ");

			if (inst.flags & x64::REP) {
				assembly.append("rep ");
			}

			if (inst.flags & x64::LOCK) {
				assembly.append("lock ");
			}

			assembly.append("{}", inst.get_mnemonic());

			if (inst.data_type_1 >= x64::SSE_SS && inst.data_type_1 <= x64::SSE_PD) {
				static const char* type_ids[] = { "ss", "sd", "ps", "pd" };
				assembly.append(type_ids[inst.data_type_1 - x64::SSE_SS]);
			}

			assembly.append(" ");

			bool mem = true;
			bool imm = true;

			for (u8 i = 0; i < 4; i++) {
				if (inst.registers[i].is_valid() == false) {
					if (mem && inst.flags & x64::USE_MEM_OP) {
						if (i > 0) {
							assembly.append(", ");
						}

						mem = false;

						if (inst.flags & x64::USE_RIP_MEM) {
							const bool is_label = 
								inst.opcode == 0xE8 || 
								inst.opcode == 0xE9 || 
								(inst.opcode >= 0x70 && inst.opcode <= 0x7F) || 
								(inst.opcode >= 0x0F80 && inst.opcode <= 0x0F8F);

							if (!is_label) {
								assembly.append("[");
							}

							if (patch && patch->pos == range.start + inst.length - 4) {
								handle<symbol> target = patch->target;

								if (target->name.empty()) {
									assembly.append("sym{}", static_cast<void*>(target.get()));
								}
								else {
									assembly.append("{}", target->name);
								}

								patch = patch->next;
							}
							else {
								const u64 target = range.start + inst.length + inst.memory.displacement;
								u64 label_basic_block = emit_get_label(context, target);
								assembly.append(".bb{}", label_basic_block);
							}

							if (!is_label) {
								assembly.append("]");
							}
						}
						else {
							assembly.append("{} [", x64::get_type_name(inst.data_type_1));

							if (inst.base != 255) {
								assembly.append("{}", get_register_name(inst.base, x64::QWORD));
							}

							if (inst.memory.index != 255) {
								assembly.append(
									" + {}*{}", get_register_name(inst.memory.index, x64::QWORD), 1 << static_cast<u8>(inst.memory.scale)
								);
							}

							if (inst.memory.displacement > 0) {
								assembly.append(" + {}", inst.memory.displacement);
							}
							else if (inst.memory.displacement < 0) {
								assembly.append(" - {}", -inst.memory.displacement);
							}

							assembly.append("]");
						}
					}
					else if (imm && (inst.flags & (x64::IMMEDIATE | x64::ABSOLUTE))) {
						if (i > 0) {
							assembly.append(", ");
						}

						imm = false;

						if (inst.flags & x64::ABSOLUTE) {
							assembly.append("{}", inst.abs);
						}
						else {
							assembly.append("{}", inst.imm);
						}
					}
					else {
						break;
					}
				}
				else {
					if (i > 0) {
						assembly.append(", ");

						// special case for certain ops with two data types
						if (inst.flags & x64::TWO_DATA_TYPES) {
							assembly.append(
								"{}", x64::get_register_name(inst.registers[i], inst.data_type_2)
							);

							continue;
						}
					}

					assembly.append(
						"{}", x64::get_register_name(inst.registers[i], inst.data_type_1)
					);
				}
			}

			assembly.append("\n");
			range.start += inst.length;
		}

		return patch;
	}

	bool x64_disassembler::disassemble_instruction(const utility::byte_buffer& bytecode, x64::x64_instruction& inst) {
		inst = { 0 };
		for(u8 i = 0; i < 4; ++i) {
			inst.registers[i] = reg::invalid_id;
		}

		u64 current = 0;

		// parse prefixes
		u8 op;
		u8 rex          = 0;     // 0x4X
		bool address_32;
		bool address_16 = false; // 0x66
		bool ext        = false; // 0x0F

		while (true) {
			if(current + 1 > bytecode.get_size()) {
				return false;
			}

			op = bytecode[current++];

			switch (op) {
				case 0x40: 
				case 0x41:
				case 0x42:
				case 0x43:
				case 0x44:
				case 0x45:
				case 0x46:
				case 0x47:
				case 0x48:
				case 0x49:
				case 0x4A:
				case 0x4B:
				case 0x4C:
				case 0x4D:
				case 0x4E:
				case 0x4F: rex = op; break;
				case 0xF0: inst.flags |= x64::LOCK; break;
				case 0x66: address_16 = true; break;
				case 0x67: address_32 = true; break;
				case 0xF3: inst.flags |= x64::REP; break;
				case 0xF2: inst.flags |= x64::REPNE; break;
				case 0x2E: inst.segment = x64::x64_segment::CS; break;
				case 0x36: inst.segment = x64::x64_segment::SS; break;
				case 0x3E: inst.segment = x64::x64_segment::DS; break;
				case 0x26: inst.segment = x64::x64_segment::ES; break;
				case 0x64: inst.segment = x64::x64_segment::FS; break;
				case 0x65: inst.segment = x64::x64_segment::GS; break;
				default: goto done_prefixing;
			}
		}

	done_prefixing:;
		if (op == 0x0F) {
			ext = true;
			op = bytecode[current++];
		}

		const static std::array<u16, 256> first_table = get_first_table();
		const static std::array<u16, 256> ext_table   = get_ext_table();

		inst.opcode = (ext ? 0x0F00 : 0) | op;
		const u16 first = ext ? ext_table[op] : first_table[op];
		u16 flags = first & 0xFFF;

		ASSERT(first != 0, "unknown op detected");

		// info from the table
		const u16 enc = first & 0xF000;
		const bool uses_imm = enc == x64::OP_MI || enc == x64::OP_MI8;

		// in the "default" "type" "system", REX.W is 64bit, certain ops
		// will mark they're 8bit and most will just be 32bit (with 16bit on ADDR16)
		inst.data_type_1 = x64::DWORD;

		if (flags & x64::OP_64BIT) {
			// basically forced 64bit
			inst.data_type_1 = x64::QWORD;
		}
		else if (flags & x64::OP_SSE) {
			// ss REP    OPCODE
			// sd REPNE  OPCODE
			// ps __     OPCODE
			// pd DATA16 OPCODE
			if (inst.flags & x64::REPNE) {
				inst.data_type_1 = x64::SSE_SD;
			}
			else if (inst.flags & x64::REP) {
				inst.data_type_1 = x64::SSE_SS;
			}
			else if (address_16) {
				inst.data_type_1 = x64::SSE_SS;
			}
			else {
				inst.data_type_1 = x64::SSE_PS;
			}

			inst.flags &= ~(x64::REP | x64::REPNE);
		}
		else {
			if (rex & 0x8) {
				inst.data_type_1 = x64::QWORD;
			}
			else if (flags & x64::OP_8BIT) {
				inst.data_type_1 = x64::BYTE;
			}
			else if (address_16) {
				inst.data_type_1 = x64::WORD;
			}
		}

		ASSERT(enc != x64::OP_BAD, "bad operation detected");

		if (enc == x64::OP_IMM) {
			if (current + 4 > bytecode.get_size()) {
				return false;
			}

			inst.flags |= x64::IMMEDIATE;
			inst.imm = bytecode.get_dword(current);
			current += 4;

			inst.length = current;
			return true;
		}

		if (enc == x64::OP_REL32) {
			inst.flags |= x64::USE_RIP_MEM;
			inst.flags |= x64::USE_MEM_OP;
			inst.base = -1;
			inst.memory.index = -1;

			if (current + 4 > bytecode.get_size()) {
				return false;
			}

			inst.memory.displacement = bytecode.get_dword(current);
			current += 4;

			inst.length = current;
			return true;
		}

		if (enc == x64::OP_0ARY) {
			inst.length = current;
			return true;
		}

		if (enc == x64::OP_PLUSR) {
			// bottom 8 bits of the opcode are the base reg
			inst.registers[0] = (rex & 1 ? 1 : 0) | (inst.opcode & 7);
			inst.opcode &= ~7;

			if (op >= 0xB8 && op <= 0xBF) {
				if (current + 8 > bytecode.get_size()) {
					return false;
				}

				inst.flags |= x64::ABSOLUTE;
				inst.abs = bytecode.get_qword(current);
				current += 8;
			}

			inst.length = current;
			return true;
		}
		
		// parse mod rm
		const bool rm_slot = enc == x64::OP_RM;
		const bool rx_slot = !rm_slot;

		if (current + 1 > bytecode.get_size()) {
			return false;
		}

		const u8 mod_rm = bytecode[current++];
		const u8 mod = mod_rm >> 6;
		const u8 rx = (mod_rm >> 3) & 7;
		const u8 rm = mod_rm & 7;

		if (flags & x64::OP_FAKERX) {
			inst.opcode <<= 4;
			inst.opcode |= rx;

			if (inst.opcode == 0xF60) {
				flags = x64::OP_MI8;
			}
			else if (inst.opcode == 0xF70) {
				flags = x64::OP_MI;
			}
		}

		if (flags & x64::OP_2DT) {
			if (inst.opcode == 0x0FB6 || inst.opcode == 0x0FB7) {
				inst.flags |= x64::TWO_DATA_TYPES;
				inst.data_type_2 = inst.opcode == 0x0FB6 ? x64::BYTE : x64::WORD;
			}
			else {
				inst.flags |= x64::TWO_DATA_TYPES;
				inst.data_type_2 = x64::DWORD;
			}
		}

		if (enc != x64::OP_M && !uses_imm) {
			if (enc == x64::OP_MC) {
				inst.flags |= x64::TWO_DATA_TYPES;
				inst.data_type_2 = x64::BYTE;
				inst.registers[rx_slot] = x64::RCX;
			}
			else {
				i8 real_rx = static_cast<i8>((rex & 4 ? 8 : 0) | rx);

				if (rex == 0 && inst.data_type_1 == x64::BYTE && real_rx >= 4) {
					// use high registers
					real_rx += 16;
				}

				inst.registers[rx_slot] = real_rx;
			}
		}

		// writes out the RM reg (or base and index)
		const ptr_diff delta = disassemble_memory_operand(
			bytecode.get_slice(current, bytecode.get_size()),
			bytecode.get_size() - current,
			rm_slot, 
			mod, 
			rm, 
			rex, 
			inst
		);

		if (delta < 0) {
			return false;
		}

		current += delta;

		// immediates might use RX for an extended opcode
		// imul's ternary is a special case
		if (uses_imm || op == 0x68 || op == 0x69) {
			if ((enc == x64::OP_MI && inst.data_type_1 == x64::BYTE) || enc == x64::OP_MI8 || op == 0x68) {
				if (current + 1 > bytecode.get_size()) {
					return false;
				}

				inst.flags |= x64::IMMEDIATE;
				inst.imm = bytecode[current++];
			}
			else if (enc == x64::OP_MI || op == 0x69) {
				if (current + 4 > bytecode.get_size()) {
					return false;
				}

				inst.flags |= x64::IMMEDIATE;
				inst.imm = static_cast<i32>(bytecode.get_dword(current));
				current += 4;
			}
			else {
				return false;
			}
		}

		inst.length = current;
		return true;
	}

	ptr_diff x64_disassembler::disassemble_memory_operand(
		const utility::byte_buffer& bytecode,
		u64 length, 
		i32 reg_slot,
		u8 mod, 
		u8 rm, 
		u8 rex,
		x64::x64_instruction& inst
	) {
		if (mod == x64::DIRECT) {
			inst.registers[reg_slot] = (rex & 1 ? 8 : 0) | rm;
			return 0;
		}

		u64 current = 0;

		inst.memory.displacement = 0;
		inst.flags |= x64::USE_MEM_OP;

		// indirect
		if (rm == x64::RSP) {
			if (current + 1 > length) {
				return 0;
			}

			const u8 sib = bytecode[current++];

			const scale s = static_cast<scale>((sib >> 6));
			const u8 index = (sib >> 3) & 7;
			const u8 base = (sib & 7);

			const u8 base_gpr = mod != x64::INDIRECT || base != x64::RSP ? ((rex & 1 ? 8 : 0) | base) : -1;
			const u8 index_gpr = mod != x64::INDIRECT || index != x64::RSP ? ((rex & 2 ? 8 : 0) | index) : -1;

			// odd rule but when mod=00,base=101,index=100
			// and using SIB, enable displacement_32. this would technically
			// apply to R13 too which means you can't do
			//   lea rax, [r13 + rcx*2] or lea rax, [rbp + rcx*2]
			// only
			//   lea rax, [r13 + rcx*2 + 0] or lea rax, [rbp + rcx*2 + 0]
			if (mod == 0 && base == x64::RBP) {
				mod = x64::INDIRECT_DISPLACEMENT_32;
			}

			inst.base = base_gpr;
			inst.memory.index = index_gpr;
			inst.memory.scale = s;
		}
		else {
			if (mod == x64::INDIRECT && rm == x64::RBP) {
				// RIP-relative addressing
				if (current + 4 > length) {
					return 0;
				}

				inst.flags |= x64::USE_RIP_MEM;
				inst.base = std::numeric_limits<u8>::max();
				inst.memory.index = -1;
				inst.memory.displacement = static_cast<i32>(bytecode.get_dword(current));
				current += 4;
			}
			else {
				inst.base = (rex & 1 ? 8 : 0) | rm;
				inst.memory.index = 255;
				inst.memory.scale = scale::x1;
			}
		}

		if (mod == x64::INDIRECT_DISPLACEMENT_8) {
			if (current + 1 > length) {
				return 0;
			}

			// NOTE: this has to be cast to i8 before we can use it
			inst.memory.displacement = (i8)bytecode[current++];
		}
		else if (mod == x64::INDIRECT_DISPLACEMENT_32) {
			if (current + 4 > length) {
				return 0;
			}

			inst.memory.displacement = static_cast<i32>(bytecode.get_dword(current));
			current += 4;
		}

		inst.registers[reg_slot] = reg::invalid_id;
		return static_cast<ptr_diff>(current);
	}

	u64 x64_disassembler::emit_get_label(const codegen_context& context, u64 position) {
		for (u64 i = 0; i < context.labels.size(); ++i) {
			ASSERT(context.labels[i] & 0x80000000, "invalid label high bit");

			if ((context.labels[i] & ~0x80000000) == position) {
				return i;
			}
		}

		return 0;
	}

	std::array<u16, 256> x64_disassembler::get_first_table() {
		std::array<u16, 256> first_table = {};

		#define GENERATE_BINARY_OP(op)                         \
		do {                                                   \
			first_table[(op) + 0] = x64::OP_MR | x64::OP_8BIT; \
			first_table[(op) + 1] = x64::OP_MR;                \
			first_table[(op) + 2] = x64::OP_RM | x64::OP_8BIT; \
			first_table[(op) + 3] = x64::OP_RM;                \
		} while (false)
		
		GENERATE_BINARY_OP(0x00); // add
		GENERATE_BINARY_OP(0x08); // or
		GENERATE_BINARY_OP(0x20); // and
		GENERATE_BINARY_OP(0x28); // sub
		GENERATE_BINARY_OP(0x30); // xor
		GENERATE_BINARY_OP(0x38); // cmp
		GENERATE_BINARY_OP(0x88); // mov

		#undef GENERATE_BINARY_OP

		first_table[0x80] = x64::OP_MI8 | x64::OP_8BIT | x64::OP_FAKERX;
		first_table[0x81] = x64::OP_MI | x64::OP_FAKERX;
		first_table[0x83] = x64::OP_MI8 | x64::OP_FAKERX;
		first_table[0x84] = x64::OP_MR | x64::OP_8BIT;
		first_table[0x85] = x64::OP_MR;
		first_table[0x8D] = x64::OP_RM;
		first_table[0x63] = x64::OP_RM | x64::OP_2DT;
		first_table[0x68] = x64::OP_IMM;
		first_table[0x69] = x64::OP_RM;
		first_table[0x90] = x64::OP_0ARY;
		first_table[0x99] = x64::OP_0ARY;
		first_table[0xC3] = x64::OP_0ARY;
		first_table[0xC6] = x64::OP_MI | x64::OP_FAKERX | x64::OP_8BIT;
		first_table[0xC7] = x64::OP_MI | x64::OP_FAKERX;
		first_table[0xC0] = x64::OP_MI8 | x64::OP_FAKERX | x64::OP_8BIT;
		first_table[0xC1] = x64::OP_MI8 | x64::OP_FAKERX;
		first_table[0xD2] = x64::OP_MC | x64::OP_FAKERX | x64::OP_8BIT;
		first_table[0xD3] = x64::OP_MC | x64::OP_FAKERX;
		first_table[0xE8] = x64::OP_REL32;
		first_table[0xE9] = x64::OP_REL32;
		first_table[0xEB] = x64::OP_REL8;
		first_table[0xF6] = x64::OP_M | x64::OP_FAKERX | x64::OP_8BIT;
		first_table[0xF7] = x64::OP_M | x64::OP_FAKERX;
		first_table[0xFF] = x64::OP_M | x64::OP_FAKERX;

		for (int i = 0x50; i <= 0x5F; ++i) {
			first_table[i] = x64::OP_PLUSR | x64::OP_64BIT;
		}

		for (int i = 0x70; i <= 0x7F; ++i) {
			first_table[i] = x64::OP_REL8;
		}

		for (int i = 0xB8; i <= 0xBF; ++i) {
			first_table[i] = x64::OP_PLUSR | x64::OP_64BIT;
		}

		for (int i = 0xA4; i <= 0xA5; ++i) {
			first_table[i] = x64::OP_0ARY;
		}

		for (int i = 0xAA; i <= 0xAB; ++i) {
			first_table[i] = x64::OP_0ARY;
		}

		for (int i = 0xAE; i <= 0xAF; ++i) {
			first_table[i] = x64::OP_0ARY;
		}

		return first_table;
	}

	std::array<u16, 256> x64_disassembler::get_ext_table() {
		std::array<u16, 256> ext_table = {};

		ext_table[0x0B] = x64::OP_0ARY;
		ext_table[0x18] = x64::OP_FAKERX;
		ext_table[0x10] = x64::OP_RM | x64::OP_SSE;
		ext_table[0x11] = x64::OP_MR | x64::OP_SSE;
		ext_table[0x2E] = x64::OP_RM | x64::OP_SSE;
		ext_table[0x1F] = x64::OP_RM;
		ext_table[0xAF] = x64::OP_RM;

		for (int i = 0x51; i <= 0x5F; ++i) {
			ext_table[i] = x64::OP_RM | x64::OP_SSE;
		}

		for (int i = 0x40; i <= 0x4F; ++i) {
			ext_table[i] = x64::OP_RM;
		}

		for (int i = 0xB6; i <= 0xB7; ++i) {
			ext_table[i] = x64::OP_RM | x64::OP_2DT;
		}

		for (int i = 0x80; i <= 0x8F; ++i) {
			ext_table[i] = x64::OP_REL32;
		}

		for (int i = 0x90; i <= 0x9F; ++i) {
			ext_table[i] = x64::OP_M;
		}

		return ext_table;
	}

	std::string x64::x64_instruction::get_mnemonic() const {
		// cwd/cdq/cqo
		if (opcode == 0x99) {
			if (data_type_1 == WORD) {
				return "cwd";
			}
			if (data_type_1 == DWORD) {
				return "cdq";
			}
			if (data_type_1 == QWORD) {
				return "cqo";
			}

			return "??";
		}

		switch (opcode) {
			case 0x0F0B: return "ud2";

			case 0x0F180: return "prefetchnta";
			case 0x0F181: return "prefetch0";
			case 0x0F182: return "prefetch1";
			case 0x0F183: return "prefetch2";

			case 0x00: case 0x01: case 0x02: case 0x03:
				return "add";
			case 0x08: case 0x09: case 0x0A: case 0x0B:
				return "or";
			case 0x20: case 0x21: case 0x22: case 0x23:
				return "and";
			case 0x28: case 0x29: case 0x2A: case 0x2B:
				return "sub";
			case 0x30: case 0x31: case 0x32: case 0x33:
				return "xor";
			case 0x38: case 0x39: case 0x3A: case 0x3B:
			case 0x80: case 0x81: case 0x82: case 0x83:
				return "cmp";
			case 0x88: case 0x89: case 0x8A: case 0x8B:
				return "mov";

			case 0xA4: case 0xA5: return "movs";
			case 0xAA: case 0xAB: return "stos";
			case 0xAE: case 0xAF: return "scas";

			case 0xC00: case 0xC10: case 0xD20: case 0xD30: return "rol";
			case 0xC01: case 0xC11: case 0xD21: case 0xD31: return "ror";
			case 0xC04: case 0xC14: case 0xD24: case 0xD34: return "shl";
			case 0xC05: case 0xC15: case 0xD25: case 0xD35: return "shr";
			case 0xC07: case 0xC17: case 0xD27: case 0xD37: return "sar";

			case 0xF60: case 0xF70: return "test";
			case 0xF66: case 0xF76: return "div";
			case 0xF67: case 0xF77: return "idiv";

			case 0x810: case 0x830: return "add";
			case 0x811: case 0x831: return "or";
			case 0x814: case 0x834: return "and";
			case 0x815: case 0x835: return "sub";
			case 0x816: case 0x836: return "xor";
			case 0x817: case 0x837: return "cmp";
			case 0xC60: case 0xC70: return "mov";
			case 0x84: case 0x85: return "test";

			case 0x0F10: case 0x0F11: return "mov";
			case 0x0F58: return "add";
			case 0x0F59: return "mul";
			case 0x0F5C: return "sub";
			case 0x0F5D: return "min";
			case 0x0F5E: return "div";
			case 0x0F5F: return "max";
			case 0x0FC2: return "cmp";
			case 0x0F2E: return "ucomi";
			case 0x0F51: return "sqrt";
			case 0x0F52: return "rsqrt";
			case 0x0F54: return "and";
			case 0x0F56: return "or";
			case 0x0F57: return "xor";

			case 0xB8: case 0xB9: case 0xBA: case 0xBB:
			case 0xBC: case 0xBD: case 0xBE: case 0xBF:
				return "mov";

			case 0x0FB6: case 0x0FB7: return "movzx";

			case 0x8D: return "lea";
			case 0x90: return "nop";
			case 0xC3: return "ret";
			case 0x63: return "movsxd";
			case 0x50: return "push";
			case 0x58: return "pop";

			case 0xE8: case 0xFF2: return "call";
			case 0xEB: case 0xE9: case 0xFF4: return "jmp";

			case 0x0F1F: return "nop";
			case 0x68: return "push";
			case 0x0FAF: case 0x69: case 0x6B: return "imul";

			case 0x0F40: return "cmovo";
			case 0x0F41: return "cmovno";
			case 0x0F42: return "cmovb";
			case 0x0F43: return "cmovnb";
			case 0x0F44: return "cmove";
			case 0x0F45: return "cmovne";
			case 0x0F46: return "cmovbe";
			case 0x0F47: return "cmova";
			case 0x0F48: return "cmovs";
			case 0x0F49: return "cmovns";
			case 0x0F4A: return "cmovp";
			case 0x0F4B: return "cmovnp";
			case 0x0F4C: return "cmovl";
			case 0x0F4D: return "cmovge";
			case 0x0F4E: return "cmovle";
			case 0x0F4F: return "cmovg";

			case 0x0F90: return "seto";
			case 0x0F91: return "setno";
			case 0x0F92: return "setb";
			case 0x0F93: return "setnb";
			case 0x0F94: return "sete";
			case 0x0F95: return "setne";
			case 0x0F96: return "setbe";
			case 0x0F97: return "seta";
			case 0x0F98: return "sets";
			case 0x0F99: return "setns";
			case 0x0F9A: return "setp";
			case 0x0F9B: return "setnp";
			case 0x0F9C: return "setl";
			case 0x0F9D: return "setge";
			case 0x0F9E: return "setle";
			case 0x0F9F: return "setg";

			case 0x0F80: case 0x70: return "jo";
			case 0x0F81: case 0x71: return "jno";
			case 0x0F82: case 0x72: return "jb";
			case 0x0F83: case 0x73: return "jnb";
			case 0x0F84: case 0x74: return "je";
			case 0x0F85: case 0x75: return "jne";
			case 0x0F86: case 0x76: return "jbe";
			case 0x0F87: case 0x77: return "ja";
			case 0x0F88: case 0x78: return "js";
			case 0x0F89: case 0x79: return "jns";
			case 0x0F8A: case 0x7A: return "jp";
			case 0x0F8B: case 0x7B: return "jnp";
			case 0x0F8C: case 0x7C: return "jl";
			case 0x0F8D: case 0x7D: return "jge";
			case 0x0F8E: case 0x7E: return "jle";
			case 0x0F8F: case 0x7F: return "jg";

			default: return "??";
		}
	}
}
