#include "x64.h"
#include "intermediate_representation/codegen/instruction.h"

#include <memory>

namespace sigma::ir {
	namespace x64 {
		auto get_register_name(reg reg, i32 dt) -> std::string {
			static const char* s_gpr_names[4][16] = {
				{ "al",  "cl",  "dl",  "bl",  "spl", "bpl", "sil", "dil", "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b" },
				{ "ax",  "cx",  "dx",  "bx",  "sp",  "bp",  "si",  "di",  "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w" },
				{ "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi", "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d" },
				{ "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi", "r8",  "r9",  "r10",  "r11",  "r12",  "r13",  "r14",  "r15"  }
			};

			if (dt >= BYTE && dt <= QWORD) {
				return s_gpr_names[dt - BYTE][reg.id];
			}

			if (dt >= SSE_SS && dt <= SSE_PD) {
				static const char* s_xmm_names[] = {
					"xmm0", "xmm1", "xmm2",  "xmm3",  "xmm4",  "xmm5",  "xmm6",  "xmm7",
					"xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
				};

				return s_xmm_names[reg.id];
			}

			return "??";
		}

		auto get_type_name(i32 dt) -> std::string {
			switch (dt) {
			case BYTE:   return "byte";
			case WORD:   return "word";
			case DWORD:
			case SSE_SS: return "dword";
			case QWORD:
			case SSE_SD: return "qword";
			case SSE_PS:
			case SSE_PD: return "xmmword";
			default:     return "??";
			}
		}
	}

	utility::byte_buffer x64_architecture::emit_bytecode(codegen_context& context) {
		resolve_stack_usage(context);

		utility::byte_buffer bytecode;

		// generate the bytecode for our function
		emit_function_prologue(context, bytecode);
		emit_function_body(context, bytecode);

		// pad the instruction buffer to 16 bytes with nop instructions
		emit_nops_to_width(bytecode);

		return bytecode;
	}

	std::vector<live_interval> x64_architecture::get_register_intervals() {
		std::vector<live_interval> intervals(32);

		for (u8 i = 0; i < 32; ++i) {
			const bool is_gpr = i < 16;

			classified_reg reg;
			reg.id = i % 16;
			reg.cl = is_gpr ? x64::register_class::GPR : x64::register_class::XMM;

			intervals[i] = live_interval{
				.reg = reg,
				.data_type = is_gpr ? x64::QWORD : x64::XMMWORD,
				.ranges = { utility::range<u64>::max() }
			};
		}

		return intervals;
	}

	void x64_architecture::resolve_stack_usage(codegen_context& context) {
		u64 caller_usage = context.caller_usage;

		if (context.target.get_abi() == abi::WIN_64 && caller_usage > 0 && caller_usage < 4) {
			caller_usage = 4;
		}

		const u64 usage = context.stack_usage + caller_usage * 8;
		context.stack_usage = utility::align(usage, 16);
	}

	void x64_architecture::emit_nops_to_width(utility::byte_buffer& bytecode) {
		// pad to 16 bytes
		static constexpr u8 nops[8][8] = {
				{ 0x90 },
				{ 0x66, 0x90 },
				{ 0x0F, 0x1F, 0x00 },
				{ 0x0F, 0x1F, 0x40, 0x00 },
				{ 0x0F, 0x1F, 0x44, 0x00, 0x00 },
				{ 0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00 },
				{ 0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00 },
				{ 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 },
		};

		u64 pad = 16 - (bytecode.get_size() & 15);

		if (pad < 16) {
			bytecode.reserve(bytecode.get_size() + pad);
			utility::byte* destination = bytecode.get_data() + bytecode.get_size();
			bytecode.set_size(bytecode.get_size() + pad);

			if (pad > 8) {
				const u64 remaining = pad - 8;
				memset(destination, 0x66, remaining);
				pad -= remaining;
				destination += remaining;
			}

			std::memcpy(destination, nops[pad - 1], pad);
		}
	}

	void x64_architecture::emit_function_prologue(codegen_context& context, utility::byte_buffer& bytecode) {
		if (context.stack_usage <= 16) {
			context.prologue_length = 0;
			return;
		}

		// push RBP
		bytecode.append_byte(0x50 + x64::gpr::RBP);

		// mov RBP, RSP
		bytecode.append_byte(rex(true, static_cast<u8>(x64::gpr::RSP), static_cast<u8>(x64::gpr::RBP), 0));
		bytecode.append_byte(0x89);
		bytecode.append_byte(mod_rx_rm(x64::DIRECT, static_cast<u8>(x64::gpr::RSP), static_cast<u8>(x64::gpr::RBP)));

		if (context.stack_usage >= 4096) {
			// emit a chkstk function
			NOT_IMPLEMENTED();
		}
		else {
			bytecode.append_byte(rex(true, 0x00, static_cast<u8>(x64::gpr::RSP), 0));

			if (utility::fits_into_8_bits(context.stack_usage)) {
				// sub RSP, stack_usage
				bytecode.append_byte(0x83);
				bytecode.append_byte(mod_rx_rm(x64::DIRECT, 0x05, static_cast<u8>(x64::gpr::RSP)));
				bytecode.append_byte(static_cast<u8>(context.stack_usage));
			}
			else {
				// sub RSP, stack_usage
				bytecode.append_byte(0x81);
				bytecode.append_byte(mod_rx_rm(x64::DIRECT, 0x05, static_cast<u8>(x64::gpr::RSP)));
				bytecode.append_dword(static_cast<u32>(context.stack_usage));
			}
		}

		context.prologue_length = static_cast<u8>(bytecode.get_size());
	}

	void x64_architecture::emit_function_body(codegen_context& context, utility::byte_buffer& bytecode) {
		for (handle<instruction> inst = context.first; inst; inst = inst->next_instruction) {
			const u8 in_base = inst->out_count;
			instruction::category cat;

			if (inst->get_type() >= s_instruction_table.size()) {
				cat = instruction::category::BINOP;
			}
			else {
				cat = s_instruction_table[inst->get_type()].category;
			}

			if (inst == instruction::type::ENTRY || inst == instruction::type::TERMINATOR) {
				/*does nothing*/
			}
			else if (inst == instruction::type::LABEL) {
				const handle<node> basic_block = inst->get<handle<node>>();
				const u32 position = static_cast<u32>(bytecode.get_size());
				const u64 id = context.graph.blocks.at(basic_block).id;

				bytecode.resolve_relocation_dword(&context.labels[id], position);
			}
			else if (inst == instruction::type::INLINE) {
				NOT_IMPLEMENTED();
			}
			else if (inst == instruction::type::EPILOGUE) {
				emit_function_epilogue(context, bytecode);

				if(inst->flags & instruction::RET) {
					bytecode.append_byte(0xC3);
				}
			}
			else if (inst == instruction::type::LINE) {
				NOT_IMPLEMENTED();
			}
			else if (cat == instruction::category::BYTE || cat == instruction::category::BYTE_EXT) {
				if (inst->get_type() & static_cast<u32>(instruction::REP)) {
					bytecode.append_byte(0xF3);
				}

				emit_instruction_0(inst->get_type(), inst->data_type, bytecode);
			}
			else if (inst->get_type() == instruction::type::ZERO) {
				const handle<instruction_operand> destination = context.create_instruction_operand();
				const bool is_xmm = inst->data_type >= x64::PBYTE && inst->data_type <= x64::XMMWORD;

				resolve_interval(context, inst, 0, destination);

				emit_instruction_2(
					context,
					is_xmm ? instruction::type::FP_XOR : instruction::type::XOR,
					destination,
					destination,
					inst->data_type,
					bytecode
				);
			}
			else if (inst->get_type() >= instruction::type::JMP && inst->get_type() <= instruction::type::JG) {
				handle<instruction_operand> target;

				if (inst->flags & instruction::NODE) {
					target = instruction_operand::create_label(context, inst->get<label>().value);
				}
				else if (inst->flags & instruction::GLOBAL) {
					NOT_IMPLEMENTED();
				}
				else {
					ASSERT(inst->in_count == 1, "");
					resolve_interval(context, inst, in_base, target);
				}

				emit_instruction_1(context, inst->get_type(), target, inst->data_type, bytecode);
			}
			else if (inst == instruction::type::CALL) {
				const handle<instruction_operand> target = context.create_instruction_operand<handle<symbol>>();
				resolve_interval(context, inst, in_base, target);
				emit_instruction_1(context, instruction::type::CALL, target, x64::QWORD, bytecode);
			}
			else {
				i32 mov_op;
				if (inst->data_type >= x64::PBYTE && inst->data_type <= x64::XMMWORD) {
					mov_op = instruction::type::FP_MOV;
				}
				else {
					mov_op = instruction::type::MOV;
				}

				// prefix
				if (inst->flags & instruction::LOCK) {
					bytecode.append_byte(0xF0);
				}

				if (inst->flags & instruction::REP) {
					bytecode.append_byte(0xF3);
				}

				i32 dt = inst->data_type;
				if (dt == x64::XMMWORD) {
					dt = x64::SSE_PD;
				}

				// resolve output
				handle<instruction_operand> out = context.create_instruction_operand();
				bool ternary = false;
				u8 resolved_operand_count = 0;

				if (inst->out_count == 1) {
					resolved_operand_count += resolve_interval(context, inst, resolved_operand_count, out);
					ASSERT(resolved_operand_count == in_base, "invalid instruction base");
				}
				else {
					resolved_operand_count = in_base;
				}

				if(inst->in_count > 0) {
					const handle<instruction_operand> left = context.create_instruction_operand();
					resolved_operand_count += resolve_interval(context, inst, resolved_operand_count, left);
					ternary = (resolved_operand_count < in_base + inst->in_count) || (inst->flags & (instruction::IMMEDIATE | instruction::ABSOLUTE));

					if (ternary && inst == instruction::type::IMUL && (inst->flags & instruction::IMMEDIATE)) {
						// there's a special case for ternary IMUL r64, r/m64, imm32
						emit_instruction_2(context, instruction::type::IMUL3, out, left, dt, bytecode);

						if (inst->data_type == x64::WORD) {
							bytecode.append_word(static_cast<u16>(inst->get<immediate>().value));
						}
						else {
							bytecode.append_dword(inst->get<immediate>().value);
						}

						continue;
					}

					if (inst->out_count == 0) {
						out = left;
					}
					else if (inst == instruction::type::IDIV || inst == instruction::type::DIV) {
						emit_instruction_1(context, inst->get_type(), left, dt, bytecode);
						continue;
					}
					else {
						if (out == nullptr) {
							out = context.create_instruction_operand();
						}

						if (ternary || inst == instruction::type::MOV || inst == instruction::type::FP_MOV) {
							if (out->matches(left) == false) {
								emit_instruction_2(context, instruction::type(static_cast<instruction::type::underlying>(mov_op)), out, left, dt, bytecode);
							}
						}
						else {
							emit_instruction_2(context, inst->get_type(), out, left, dt, bytecode);
						}
					}
				}

				// unary ops
				if (cat == instruction::category::UNARY || cat == instruction::category::UNARY_EXT) {
					emit_instruction_1(context, inst->get_type(), out, dt, bytecode);
					continue;
				}

				if(inst->flags & instruction::IMMEDIATE) {
					const handle<instruction_operand> right = instruction_operand::create_imm(context, inst->get<immediate>().value);
					emit_instruction_2(context, inst->get_type(), out, right, dt, bytecode);
				}
				else if(inst->flags & instruction::ABSOLUTE) {
					const handle<instruction_operand> right = instruction_operand::create_abs(context, inst->get<absolute>().value);
					emit_instruction_2(context, inst->get_type(), out, right, dt, bytecode);
				}
				else if(ternary) {
					const handle<instruction_operand> right = context.create_instruction_operand();
					resolved_operand_count += resolve_interval(context, inst, resolved_operand_count, right);
				
					if(inst != instruction::type::MOV || (inst == instruction::type::MOV && out->matches(right) == false)) {
						emit_instruction_2(context, inst->get_type(), out, right, dt, bytecode);
					}
				}
			}
		}
	}

	void x64_architecture::emit_function_epilogue(const codegen_context& context, utility::byte_buffer& bytecode) {
		ASSERT(context.function->exit_node != nullptr, "no exit node found");

		if (context.stack_usage <= 16) {
			return;
		}

		// add RSP, stack_usage
		bytecode.append_byte(rex(true, 0x00, static_cast<u8>(x64::gpr::RSP), 0));

		if (utility::fits_into_8_bits(context.stack_usage)) {
			bytecode.append_byte(0x83);
			bytecode.append_byte(mod_rx_rm(x64::DIRECT, 0x00, static_cast<u8>(x64::gpr::RSP)));
			bytecode.append_byte(static_cast<i8>(context.stack_usage));
		}
		else {
			bytecode.append_byte(0x81);
			bytecode.append_byte(mod_rx_rm(x64::DIRECT, 0x00, static_cast<u8>(x64::gpr::RSP)));
			bytecode.append_dword(static_cast<u32>(context.stack_usage));
		}

		// pop RBP
		bytecode.append_byte(0x58 + x64::gpr::RBP);
	}

	auto x64_architecture::get_instruction_table()->std::array<instruction::description, 120> {
		std::array<instruction::description, 120> table = {};

		table[instruction::type::RET] = { .mnemonic = "ret",       .category = instruction::category::BYTE,     .op = 0xC3 };
		table[instruction::type::INT3] = { .mnemonic = "int3",      .category = instruction::category::BYTE,     .op = 0xCC };
		table[instruction::type::NOP] = { .mnemonic = "nop",      .category = instruction::category::BYTE,     .op = 0x90 };
		table[instruction::type::STOSB] = { .mnemonic = "rep stosb", .category = instruction::category::BYTE,     .op = 0xAA };
		table[instruction::type::MOVSB] = { .mnemonic = "rep movsb", .category = instruction::category::BYTE,     .op = 0xA4 };
		table[instruction::type::CAST] = { .mnemonic = "cvt",       .category = instruction::category::BYTE,     .op = 0x99 };
		table[instruction::type::SYS_CALL] = { .mnemonic = "syscall",   .category = instruction::category::BYTE_EXT, .op = 0x05 };
		table[instruction::type::RDTSC] = { .mnemonic = "rdtsc",     .category = instruction::category::BYTE_EXT, .op = 0x31 };
		table[instruction::type::UD2] = { .mnemonic = "ud2",       .category = instruction::category::BYTE_EXT, .op = 0x0B };

		table[instruction::type::NOT] = { .mnemonic = "not",  .category = instruction::category::UNARY, .op = 0xF7, .rx_i = 0x02 };
		table[instruction::type::NEG] = { .mnemonic = "neg",  .category = instruction::category::UNARY, .op = 0xF7, .rx_i = 0x03 };
		table[instruction::type::MUL] = { .mnemonic = "mul",  .category = instruction::category::UNARY, .op = 0xF7, .rx_i = 0x04 };
		table[instruction::type::DIV] = { .mnemonic = "div",  .category = instruction::category::UNARY, .op = 0xF7, .rx_i = 0x06 };
		table[instruction::type::IDIV] = { .mnemonic = "idiv", .category = instruction::category::UNARY, .op = 0xF7, .rx_i = 0x07 };
		table[instruction::type::CALL] = { .mnemonic = "call", .category = instruction::category::UNARY, .op = 0xE8, .rx_i = 0x02 };
		table[instruction::type::JMP] = { .mnemonic = "jmp",  .category = instruction::category::UNARY, .op = 0xE9, .rx_i = 0x04 };

		// prefetching
		table[instruction::type::PREFETCHNTA] = { .mnemonic = "prefetchnta", .category = instruction::category::UNARY_EXT, .op = 0x18, .rx_i = 0 };
		table[instruction::type::PREFETCH0] = { .mnemonic = "prefetch0",   .category = instruction::category::UNARY_EXT, .op = 0x18, .rx_i = 1 };
		table[instruction::type::PREFETCH1] = { .mnemonic = "prefetch1",   .category = instruction::category::UNARY_EXT, .op = 0x18, .rx_i = 2 };
		table[instruction::type::PREFETCH2] = { .mnemonic = "prefetch2",   .category = instruction::category::UNARY_EXT, .op = 0x18, .rx_i = 3 };

		// jcc
		table[instruction::type::JO] = { .mnemonic = "jo",  .category = instruction::category::UNARY_EXT, .op = 0x80 };
		table[instruction::type::JNO] = { .mnemonic = "jno", .category = instruction::category::UNARY_EXT, .op = 0x81 };
		table[instruction::type::JB] = { .mnemonic = "jb",  .category = instruction::category::UNARY_EXT, .op = 0x82 };
		table[instruction::type::JNB] = { .mnemonic = "jnb", .category = instruction::category::UNARY_EXT, .op = 0x83 };
		table[instruction::type::JE] = { .mnemonic = "je",  .category = instruction::category::UNARY_EXT, .op = 0x84 };
		table[instruction::type::JNE] = { .mnemonic = "jne", .category = instruction::category::UNARY_EXT, .op = 0x85 };
		table[instruction::type::JBE] = { .mnemonic = "jbe", .category = instruction::category::UNARY_EXT, .op = 0x86 };
		table[instruction::type::JA] = { .mnemonic = "ja",  .category = instruction::category::UNARY_EXT, .op = 0x87 };
		table[instruction::type::JS] = { .mnemonic = "js",  .category = instruction::category::UNARY_EXT, .op = 0x88 };
		table[instruction::type::JNS] = { .mnemonic = "jns", .category = instruction::category::UNARY_EXT, .op = 0x89 };
		table[instruction::type::JP] = { .mnemonic = "jp",  .category = instruction::category::UNARY_EXT, .op = 0x8A };
		table[instruction::type::JNP] = { .mnemonic = "jnp", .category = instruction::category::UNARY_EXT, .op = 0x8B };
		table[instruction::type::JL] = { .mnemonic = "jl",  .category = instruction::category::UNARY_EXT, .op = 0x8C };
		table[instruction::type::JGE] = { .mnemonic = "jge", .category = instruction::category::UNARY_EXT, .op = 0x8D };
		table[instruction::type::JLE] = { .mnemonic = "jle", .category = instruction::category::UNARY_EXT, .op = 0x8E };
		table[instruction::type::JG] = { .mnemonic = "jg",  .category = instruction::category::UNARY_EXT, .op = 0x8F };

		// setcc
		table[instruction::type::SETO] = { .mnemonic = "seto",  .category = instruction::category::UNARY_EXT, .op = 0x90 };
		table[instruction::type::SETNO] = { .mnemonic = "setno", .category = instruction::category::UNARY_EXT, .op = 0x91 };
		table[instruction::type::SETB] = { .mnemonic = "setb",  .category = instruction::category::UNARY_EXT, .op = 0x92 };
		table[instruction::type::SETNB] = { .mnemonic = "setnb", .category = instruction::category::UNARY_EXT, .op = 0x93 };
		table[instruction::type::SETE] = { .mnemonic = "sete",  .category = instruction::category::UNARY_EXT, .op = 0x94 };
		table[instruction::type::SETNE] = { .mnemonic = "setne", .category = instruction::category::UNARY_EXT, .op = 0x95 };
		table[instruction::type::SETBE] = { .mnemonic = "setbe", .category = instruction::category::UNARY_EXT, .op = 0x96 };
		table[instruction::type::SETA] = { .mnemonic = "seta",  .category = instruction::category::UNARY_EXT, .op = 0x97 };
		table[instruction::type::SETS] = { .mnemonic = "sets",  .category = instruction::category::UNARY_EXT, .op = 0x98 };
		table[instruction::type::SETNS] = { .mnemonic = "setns", .category = instruction::category::UNARY_EXT, .op = 0x99 };
		table[instruction::type::SETP] = { .mnemonic = "setp",  .category = instruction::category::UNARY_EXT, .op = 0x9A };
		table[instruction::type::SETNP] = { .mnemonic = "setnp", .category = instruction::category::UNARY_EXT, .op = 0x9B };
		table[instruction::type::SETL] = { .mnemonic = "setl",  .category = instruction::category::UNARY_EXT, .op = 0x9C };
		table[instruction::type::SETGE] = { .mnemonic = "setge", .category = instruction::category::UNARY_EXT, .op = 0x9D };
		table[instruction::type::SETLE] = { .mnemonic = "setle", .category = instruction::category::UNARY_EXT, .op = 0x9E };
		table[instruction::type::SETG] = { .mnemonic = "setg",  .category = instruction::category::UNARY_EXT, .op = 0x9F };

		// cmovcc
		table[instruction::type::CMOVO] = { .mnemonic = "cmovo",  .category = instruction::category::BINOP_EXT_1, .op = 0x40 };
		table[instruction::type::CMOVNO] = { .mnemonic = "cmovno", .category = instruction::category::BINOP_EXT_1, .op = 0x41 };
		table[instruction::type::CMOVB] = { .mnemonic = "cmovb",  .category = instruction::category::BINOP_EXT_1, .op = 0x42 };
		table[instruction::type::CMOVNB] = { .mnemonic = "cmovnb", .category = instruction::category::BINOP_EXT_1, .op = 0x43 };
		table[instruction::type::CMOVE] = { .mnemonic = "cmove",  .category = instruction::category::BINOP_EXT_1, .op = 0x44 };
		table[instruction::type::CMOVNE] = { .mnemonic = "cmovne", .category = instruction::category::BINOP_EXT_1, .op = 0x45 };
		table[instruction::type::CMOVBE] = { .mnemonic = "cmovbe", .category = instruction::category::BINOP_EXT_1, .op = 0x46 };
		table[instruction::type::CMOVA] = { .mnemonic = "cmova",  .category = instruction::category::BINOP_EXT_1, .op = 0x47 };
		table[instruction::type::CMOVS] = { .mnemonic = "cmovs",  .category = instruction::category::BINOP_EXT_1, .op = 0x48 };
		table[instruction::type::CMOVNS] = { .mnemonic = "cmovns", .category = instruction::category::BINOP_EXT_1, .op = 0x49 };
		table[instruction::type::CMOVP] = { .mnemonic = "cmovp",  .category = instruction::category::BINOP_EXT_1, .op = 0x4A };
		table[instruction::type::CMOVNP] = { .mnemonic = "cmovnp", .category = instruction::category::BINOP_EXT_1, .op = 0x4B };
		table[instruction::type::CMOVL] = { .mnemonic = "cmovl",  .category = instruction::category::BINOP_EXT_1, .op = 0x4C };
		table[instruction::type::CMOVGE] = { .mnemonic = "cmovge", .category = instruction::category::BINOP_EXT_1, .op = 0x4D };
		table[instruction::type::CMOVLE] = { .mnemonic = "cmovle", .category = instruction::category::BINOP_EXT_1, .op = 0x4E };
		table[instruction::type::CMOVG] = { .mnemonic = "cmovg",  .category = instruction::category::BINOP_EXT_1, .op = 0x4F };

		// bit magic
		table[instruction::type::BSF] = { .mnemonic = "bsf", .category = instruction::category::BINOP_EXT_1, .op = 0xBC };
		table[instruction::type::BSF] = { .mnemonic = "bsr", .category = instruction::category::BINOP_EXT_1, .op = 0xBD };

		// binary ops but they have an implicit CL on the right-hand side
		table[instruction::type::SHL] = { .mnemonic = "shl", .category = instruction::category::BINOP_CL, .op = 0xD2, .op_i = 0xC0, .rx_i = 0x04 };
		table[instruction::type::SHR] = { .mnemonic = "shr", .category = instruction::category::BINOP_CL, .op = 0xD2, .op_i = 0xC0, .rx_i = 0x05 };
		table[instruction::type::ROL] = { .mnemonic = "rol", .category = instruction::category::BINOP_CL, .op = 0xD2, .op_i = 0xC0, .rx_i = 0x00 };
		table[instruction::type::ROR] = { .mnemonic = "ror", .category = instruction::category::BINOP_CL, .op = 0xD2, .op_i = 0xC0, .rx_i = 0x01 };
		table[instruction::type::SAR] = { .mnemonic = "sar", .category = instruction::category::BINOP_CL, .op = 0xD2, .op_i = 0xC0, .rx_i = 0x07 };

		table[instruction::type::ADD] = { .mnemonic = "add",  .category = instruction::category::BINOP, .op = 0x00, .op_i = 0x80, .rx_i = 0x00 };
		table[instruction::type::OR] = { .mnemonic = "or",   .category = instruction::category::BINOP, .op = 0x08, .op_i = 0x80, .rx_i = 0x01 };
		table[instruction::type::AND] = { .mnemonic = "and",  .category = instruction::category::BINOP, .op = 0x20, .op_i = 0x80, .rx_i = 0x04 };
		table[instruction::type::SUB] = { .mnemonic = "sub",  .category = instruction::category::BINOP, .op = 0x28, .op_i = 0x80, .rx_i = 0x05 };
		table[instruction::type::XOR] = { .mnemonic = "xor",  .category = instruction::category::BINOP, .op = 0x30, .op_i = 0x80, .rx_i = 0x06 };
		table[instruction::type::CMP] = { .mnemonic = "cmp",  .category = instruction::category::BINOP, .op = 0x38, .op_i = 0x80, .rx_i = 0x07 };
		table[instruction::type::MOV] = { .mnemonic = "mov",  .category = instruction::category::BINOP, .op = 0x88, .op_i = 0xC6, .rx_i = 0x00 };
		table[instruction::type::TEST] = { .mnemonic = "test", .category = instruction::category::BINOP, .op = 0x84, .op_i = 0xF6, .rx_i = 0x00 };

		// misc integer ops
		table[instruction::type::MOVABS] = { .mnemonic = "mov",    .category = instruction::category::BINOP_PLUS, .op = 0xB8 };
		table[instruction::type::XCHG] = { .mnemonic = "xchg",   .category = instruction::category::BINOP,      .op = 0x86 };
		table[instruction::type::LEA] = { .mnemonic = "lea",    .category = instruction::category::BINOP,      .op = 0x8D };
		table[instruction::type::XADD] = { .mnemonic = "xadd",   .category = instruction::category::BINOP_EXT_1,  .op = 0xC0 };
		table[instruction::type::IMUL] = { .mnemonic = "imul",   .category = instruction::category::BINOP_EXT_1,  .op = 0xAF };
		table[instruction::type::IMUL3] = { .mnemonic = "imul",   .category = instruction::category::BINOP,      .op = 0x69 };
		table[instruction::type::MOVSXB] = { .mnemonic = "movsxb", .category = instruction::category::BINOP_EXT_2, .op = 0xBE };
		table[instruction::type::MOVSXW] = { .mnemonic = "movsxw", .category = instruction::category::BINOP_EXT_2, .op = 0xBF };
		table[instruction::type::MOVSXD] = { .mnemonic = "movsxd", .category = instruction::category::BINOP,      .op = 0x63 };
		table[instruction::type::MOVZXB] = { .mnemonic = "movzxb", .category = instruction::category::BINOP_EXT_2, .op = 0xB6 };
		table[instruction::type::MOVZXW] = { .mnemonic = "movzxw", .category = instruction::category::BINOP_EXT_2, .op = 0xB7 };

		// gpr<->xmm
		table[instruction::type::MOV_I2F] = { .mnemonic = "mov", .category = instruction::category::BINOP_EXT_3, .op = 0x6E };
		table[instruction::type::MOV_F2I] = { .mnemonic = "mov", .category = instruction::category::BINOP_EXT_3, .op = 0x7E };

		// SSE binary operations
		table[instruction::type::FP_MOV] = { .mnemonic = "mov",   .category = instruction::category::BINOP_SSE, .op = 0x10 };
		table[instruction::type::FP_ADD] = { .mnemonic = "add",   .category = instruction::category::BINOP_SSE, .op = 0x58 };
		table[instruction::type::FP_MUL] = { .mnemonic = "mul",   .category = instruction::category::BINOP_SSE, .op = 0x59 };
		table[instruction::type::FP_SUB] = { .mnemonic = "sub",   .category = instruction::category::BINOP_SSE, .op = 0x5C };
		table[instruction::type::FP_MIN] = { .mnemonic = "min",   .category = instruction::category::BINOP_SSE, .op = 0x5D };
		table[instruction::type::FP_DIV] = { .mnemonic = "div",   .category = instruction::category::BINOP_SSE, .op = 0x5E };
		table[instruction::type::FP_MAX] = { .mnemonic = "max",   .category = instruction::category::BINOP_SSE, .op = 0x5F };
		table[instruction::type::FP_CMP] = { .mnemonic = "cmp",   .category = instruction::category::BINOP_SSE, .op = 0xC2 };
		table[instruction::type::FP_UCOMI] = { .mnemonic = "ucomi", .category = instruction::category::BINOP_SSE, .op = 0x2E };
		table[instruction::type::FP_CVT32] = { .mnemonic = "cvtsi", .category = instruction::category::BINOP_SSE, .op = 0x2A };
		table[instruction::type::FP_CVT64] = { .mnemonic = "cvtsi", .category = instruction::category::BINOP_SSE, .op = 0x2A };
		table[instruction::type::FP_CVT] = { .mnemonic = "cvt",   .category = instruction::category::BINOP_SSE, .op = 0x5A };
		table[instruction::type::FP_CVTT] = { .mnemonic = "rsqrt", .category = instruction::category::BINOP_SSE, .op = 0x2C };
		table[instruction::type::FP_SQRT] = { .mnemonic = "and",   .category = instruction::category::BINOP_SSE, .op = 0x51 };
		table[instruction::type::FP_RSQRT] = { .mnemonic = "or",    .category = instruction::category::BINOP_SSE, .op = 0x52 };
		table[instruction::type::FP_AND] = { .mnemonic = "xor",   .category = instruction::category::BINOP_SSE, .op = 0x54 };
		table[instruction::type::FP_OR] = { .mnemonic = "or",    .category = instruction::category::BINOP_SSE, .op = 0x56 };
		table[instruction::type::FP_XOR] = { .mnemonic = "xor",   .category = instruction::category::BINOP_SSE, .op = 0x57 };

		return table;
	}

	void x64_architecture::emit_instruction_0(instruction::type type, i32 data_type, utility::byte_buffer& bytecode) {
		ASSERT(type < s_instruction_table.size(), "type is out of range");
		const auto& description = &s_instruction_table[type];

		if (data_type == x64::QWORD) {
			bytecode.append_byte(0x48);
		}

		if (description->category == instruction::category::BYTE_EXT) {
			bytecode.append_byte(0x0F);
		}

		if (description->op) {
			bytecode.append_byte(description->op);
		}
		else {
			bytecode.append_byte(description->op);
			bytecode.append_byte(description->rx_i);
		}
	}

	void x64_architecture::emit_instruction_1(codegen_context& context, instruction::type type, handle<instruction_operand> r, i32 dt, utility::byte_buffer& bytecode) {
		ASSERT(type < s_instruction_table.size(), "invalid type");
		const instruction::description& descriptor = s_instruction_table[type];

		const bool is_rex = dt == x64::BYTE || dt == x64::QWORD;
		const bool is_rexw = dt == x64::QWORD;
		const u8 op = descriptor.op_i;
		const u8 rx = descriptor.rx_i;

		if (r == instruction_operand::type::GPR) {
			if (is_rex || r->reg >= 8) {
				bytecode.append_byte(rex(is_rexw, 0x00, r->reg, 0x00));
			}

			if (descriptor.category == instruction::category::UNARY_EXT) {
				bytecode.append_byte(0x0F);
			}

			bytecode.append_byte(op ? op : descriptor.op);
			bytecode.append_byte(mod_rx_rm(x64::DIRECT, rx, r->reg));
		}
		else if (r == instruction_operand::type::MEM) {
			const i32 displacement = r->immediate;
			const u8 index = r->index;
			const u8 base = r->reg;
			memory_scale s = r->scale;

			const bool needs_index = (index != reg::invalid_id) || (base & 7) == x64::gpr::RSP;
			bytecode.append_byte(rex(is_rexw, 0x00, base, index != reg::invalid_id ? index : 0));

			if (descriptor.category == instruction::category::UNARY_EXT) {
				bytecode.append_byte(0x0F);
			}

			bytecode.append_byte(op);

			// if it needs an index, it'll put RSP into the base slot
			// and write the real base into the SIB
			x64::mod m = x64::INDIRECT_DISPLACEMENT_32;

			if (utility::fits_into_8_bits(displacement)) {
				m = x64::INDIRECT_DISPLACEMENT_8;
			}

			bytecode.append_byte(mod_rx_rm(m, rx, needs_index ? static_cast<u8>(x64::gpr::RSP) : base));

			if (needs_index) {
				bytecode.append_byte(mod_rx_rm(static_cast<x64::mod>(s), (base & 7) == x64::gpr::RSP ? static_cast<u8>(x64::gpr::RSP) : index, base));
			}

			if (m == x64::INDIRECT_DISPLACEMENT_8) {
				bytecode.append_byte(static_cast<i8>(displacement));
			}
			else if (m == x64::INDIRECT_DISPLACEMENT_32) {
				bytecode.append_dword(static_cast<i32>(displacement));
			}
		}
		else if (r == instruction_operand::type::GLOBAL) {
			if (descriptor.op) {
				if (descriptor.category == instruction::category::UNARY_EXT) {
					bytecode.append_byte(0x0F);
				}

				bytecode.append_byte(descriptor.op);
			}
			else {
				if (is_rex) {
					bytecode.append_byte(is_rexw ? 0x48 : 0x40);
				}

				if (descriptor.category == instruction::category::UNARY_EXT) {
					bytecode.append_byte(0x0F);
				}

				bytecode.append_byte(op);
				bytecode.append_byte(((rx & 7) << 3) | x64::gpr::RBP);
			}

			bytecode.append_dword(r->immediate);
			emit_symbol_patch(context, r->get<handle<symbol>>(), bytecode.get_size() - 4);
		}
		else if (r == instruction_operand::type::LABEL) {
			if (descriptor.category == instruction::category::UNARY_EXT) {
				bytecode.append_byte(0x0F);
			}

			bytecode.append_byte(descriptor.op);
			bytecode.append_dword(0);

			const u64 label = r->get<ir::label>().value;
			bytecode.emit_relocation_dword(&context.labels[label], static_cast<u32>(bytecode.get_size()) - 4);
		}
		else {
			NOT_IMPLEMENTED();
		}
	}

	void x64_architecture::emit_instruction_2(codegen_context& context, instruction::type type, handle<instruction_operand> a, handle<instruction_operand> b, i32 data_type, utility::byte_buffer& bytecode) {
		if (data_type >= x64::SSE_SS && data_type <= x64::SSE_PD) {
			NOT_IMPLEMENTED();
		}

		ASSERT(data_type >= x64::BYTE && data_type <= x64::QWORD, "invalid data type");
		ASSERT(type < s_instruction_table.size(), "invalid type");

		const instruction::description& descriptor = s_instruction_table[type];
		const bool dir = b == instruction_operand::type::MEM || b == instruction_operand::type::GLOBAL;

		if (type == instruction::type::MOVABS) {
			ASSERT(
				a == instruction_operand::type::GPR && b == instruction_operand::type::ABS,
				"invalid data types for a movabs operation"
			);

			bytecode.append_byte(rex(true, a->reg, 0, 0));
			bytecode.append_byte(descriptor.op + (a->reg & 0b111));
			bytecode.append_qword(b->get<absolute>().value);
			return;
		}

		if (dir ||
			descriptor.op == 0x63 ||
			descriptor.op == 0x69 ||
			descriptor.op == 0x6E ||
			(type >= instruction::type::CMOVO && type <= instruction::type::CMOVG) ||
			descriptor.op == 0xAF ||
			descriptor.category == instruction::category::BINOP_EXT_2
			) {
			std::swap(a, b);
		}

		// operand size 
		bool sz = data_type != x64::BYTE;

		// uses an imm value that works as a sign extended 8 bit number
		const bool short_imm = sz && b == instruction_operand::type::IMM && b->immediate == static_cast<i8>(b->immediate) && descriptor.op_i == 0x80;

		// the destination can only be a GPR, no direction flag
		const bool is_gpr_only_dst = descriptor.op & 1;
		const bool dir_flag = dir != is_gpr_only_dst && descriptor.op != 0x69;

		if (descriptor.category != instruction::category::BINOP_EXT_3) {
			// address size prefix
			if (data_type == x64::WORD && descriptor.category != instruction::category::BINOP_EXT_2) {
				bytecode.append_byte(0x66);
			}

			ASSERT(b == instruction_operand::type::GPR || b == instruction_operand::type::IMM, "secondary operand is invalid!");
		}
		else {
			bytecode.append_byte(0x66);
		}

		// REX PREFIX
		//  0 1 0 0 W R X B
		//          ^ ^ ^ ^
		//          | | | 4th bit on base
		//          | | 4th bit on index
		//          | 4th bit on rx
		//          is 64bit?

		u8 rex_prefix = 0x40 | (data_type == x64::QWORD ? 8 : 0);
		u8 base;

		if (a == instruction_operand::type::MEM || a == instruction_operand::type::GPR) {
			base = a->reg;
		}
		else {
			base = static_cast<u8>(x64::gpr::RBP);
		}

		if (a == instruction_operand::type::MEM && a->index != reg::invalid_id) {
			rex_prefix |= ((a->index >> 3) << 1);
		}

		u8 rx;
		if (b == instruction_operand::type::GPR || b == instruction_operand::type::XMM) {
			rx = b->reg;
		}
		else {
			rx = descriptor.rx_i;
		}

		if (descriptor.category == instruction::category::BINOP_CL) {
			ASSERT(b == instruction_operand::type::IMM || (b == instruction_operand::type::GPR && b->reg == x64::gpr::RCX), "invalid binary operation");

			data_type = x64::BYTE;
			rx = descriptor.rx_i;
		}

		rex_prefix |= (base >> 3);
		rex_prefix |= (rx >> 3) << 2;

		// if the REX stays as 0x40 then it's default and doesn't need to be here
		if (rex_prefix != 0x40 || data_type == x64::BYTE || type == instruction::type::MOVZXB) {
			bytecode.append_byte(rex_prefix);
		}

		if (descriptor.category == instruction::category::BINOP_EXT_3) {
			bytecode.append_byte(0x0F);
			bytecode.append_byte(descriptor.op);
		}
		else {
			// opcode
			if (descriptor.category == instruction::category::BINOP_EXT_1 || descriptor.category == instruction::category::BINOP_EXT_2) {
				// DEF instructions can only be 32bit and 64bit... maybe?
				if (type != instruction::type::XADD) {
					sz = false;
				}

				bytecode.append_byte(0x0F);
			}

			// immediates have a custom opcode
			ASSERT(b != instruction_operand::type::IMM || descriptor.op_i != 0 || descriptor.rx_i != 0, "no immediate variant of instruction");
			u8 opcode = b->get_type() == instruction_operand::type::IMM ? descriptor.op_i : descriptor.op;

			// the bottom bit usually means size, 0 for 8bit, 1 for everything else
			opcode |= static_cast<u8>(sz);

			// you can't actually be flipped in the immediates because it would mean
			// you're storing into an immediate so they reuse that direction bit for size
			opcode |= dir_flag << 1;
			opcode |= short_imm << 1;

			bytecode.append_byte(opcode);
		}

		emit_memory_operand(context, rx, a, bytecode);

		// memory displacements go before immediates
		const u64 disp_patch = bytecode.get_size() - 4;

		if (b == instruction_operand::type::IMM) {
			if (data_type == x64::BYTE || short_imm) {
				if (short_imm) {
					ASSERT(b->immediate == static_cast<i8>(b->immediate), "invalid short immediate");
				}

				bytecode.append_byte(static_cast<i8>(b->immediate));
			}
			else if (data_type == x64::WORD) {
				const i32 imm = b->immediate;
				ASSERT((imm & 0xFFFF0000) == 0xFFFF0000 || (imm & 0xFFFF0000) == 0, "invalid immediate");
				bytecode.append_word(static_cast<u16>(imm));
			}
			else {
				bytecode.append_dword(b->immediate);
			}
		}

		if (a == instruction_operand::type::GLOBAL && disp_patch + 4 != bytecode.get_size()) {
			bytecode.patch_dword(disp_patch, static_cast<u32>(disp_patch + 4 - bytecode.get_size()));
		}
	}

	void x64_architecture::emit_memory_operand(codegen_context& context, u8 rx, handle<instruction_operand> a, utility::byte_buffer& bytecode) {
		// operand encoding
		if (a == instruction_operand::type::GPR || a == instruction_operand::type::XMM) {
			bytecode.append_byte(mod_rx_rm(x64::DIRECT, rx, a->reg));
		}
		else if (a == instruction_operand::type::MEM) {
			const u8 base = a->reg;
			const u8 index = a->index;
			memory_scale scale = a->scale;
			const i32 displacement = a->immediate;
			const bool needs_index = (index != reg::invalid_id) || (base & 7) == x64::gpr::RSP;

			// if it needs an index, it'll put RSP into the base slot and write the real base
			// into the SIB
			x64::mod m = x64::INDIRECT_DISPLACEMENT_32;

			if (displacement == 0 && (base & 7) != x64::gpr::RBP) {
				m = x64::INDIRECT;
			}
			else if (displacement == static_cast<i8>(displacement)) {
				m = x64::INDIRECT_DISPLACEMENT_8;
			}

			bytecode.append_byte(mod_rx_rm(m, rx, needs_index ? static_cast<u8>(x64::gpr::RSP) : base));

			if (needs_index) {
				bytecode.append_byte(mod_rx_rm(static_cast<x64::mod>(scale), (base & 7) == x64::gpr::RSP ? static_cast<u8>(x64::gpr::RSP) : index, base));
			}

			if (m == x64::INDIRECT_DISPLACEMENT_8) {
				bytecode.append_byte(static_cast<i8>(displacement));
			}
			else if (m == x64::INDIRECT_DISPLACEMENT_32) {
				bytecode.append_dword(displacement);
			}
		}
		else if (a == instruction_operand::type::GLOBAL) {
			bytecode.append_byte(((rx & 7) << 3) | x64::gpr::RBP);
			bytecode.append_dword(a->immediate);
			emit_symbol_patch(context, a->get<handle<symbol>>(), bytecode.get_size() - 4);
		}
		else {
			NOT_IMPLEMENTED();
		}
	}

	auto x64_architecture::resolve_interval(const codegen_context& context, handle<instruction> inst, u8 i, handle<instruction_operand> val) -> u8 {
		handle interval = &context.intervals[inst->operands[i]];

		if((inst->flags & (instruction::MEM | instruction::GLOBAL)) && i == inst->memory.index) {
			ASSERT(interval->spill <= 0, "cannot use spilled value for a memory operand");

			if(inst->flags & instruction::MEM) {
				val->set_type(instruction_operand::type::MEM);
				val->reg = interval->assigned.id;
				val->index = reg::invalid_id;
				val->scale = inst->memory.scale;
				val->immediate = inst->memory.displacement;

				if(inst->flags & instruction::INDEXED) {
					interval = &context.intervals[inst->operands[i + 1]];
					ASSERT(interval->spill <= 0, "cannot use spilled value for a memory operand");

					val->index = interval->assigned.id;
					return 2;
				}
				else {
					return 1;
				}
			}
			else {
				val->set_type(instruction_operand::type::GLOBAL);
				val->immediate = inst->memory.displacement;
				val->get<handle<symbol>>() = inst->get<handle<symbol>>();
				return 1;
			}
		}

		if(interval->spill > 0) {
			val->set_type(instruction_operand::type::MEM);
			val->reg = static_cast<u8>(x64::gpr::RBP);

			val->index = reg::invalid_id;
			val->immediate = -interval->spill;
		}
		else {
			if(interval->reg.cl == x64::register_class::XMM) {
				val->set_type(instruction_operand::type::XMM);
			}
			else {
				val->set_type(instruction_operand::type::GPR);
			}

			val->reg = interval->assigned.id;
		}

		return 1;
	}

	void x64_architecture::emit_symbol_patch(codegen_context& context, handle<symbol> target, u64 pos) {
		const handle<symbol_patch> patch = context.create_symbol_patch();

		patch->target = target;
		patch->next = nullptr;
		patch->pos = pos;

		context.patch_count++;

		if (context.first_patch == nullptr) {
			context.first_patch = context.last_patch = patch;
		}
		else {
			context.last_patch->next = patch;
			context.last_patch = patch;
		}
	}

	auto rex(bool is_64_bit, u8 rx, u8 base, u8 index) -> u8 {
		return 0x40 | (is_64_bit ? 8 : 0) | (base >> 3) | ((index >> 3) << 1) | ((rx >> 3) << 2);
	}

	auto mod_rx_rm(x64::mod mod, u8 rx, u8 rm) -> u8 {
		return ((mod & 3) << 6) | ((rx & 7) << 3) | (rm & 7);
	}
} // namespace sigma::ir
