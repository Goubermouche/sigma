#include "x64.h"

#include "intermediate_representation/codegen/instruction.h"

namespace sigma::ir {
	utility::byte_buffer x64_architecture::emit_bytecode(codegen_context& context) {
		resolve_stack_usage(context);

		utility::byte_buffer bytecode;

		// generate the bytecode for our function
		emit_function_prologue(context, bytecode);
		emit_function_body(context, bytecode);
		emit_function_epilogue(context, bytecode);

		pad(bytecode);

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
				.r = reg,
				.data_type = is_gpr ? x64::QWORD : x64::XMMWORD,
				.ranges = { utility::range<u64>::max() }
			};
		}

		return intervals;
	}

	void x64_architecture::resolve_stack_usage(codegen_context& context) {
		u64 caller_usage = context.caller_usage;

		if(context.t.get_abi() == abi::WIN_64 && caller_usage > 0 && caller_usage < 4) {
			caller_usage = 4;
		}

		const u64 usage = context.stack_usage + caller_usage * 8;
		context.stack_usage = utility::align(usage, 16);
	}

	void x64_architecture::pad(utility::byte_buffer& bytecode) {
		// pad to 16bytes
		static const u8 nops[8][8] = {
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

		if(pad < 16) {
			bytecode.reserve(bytecode.get_size() + pad);
			utility::byte* dst = bytecode.get_data() + bytecode.get_size();
			bytecode.set_size(bytecode.get_size() + pad);

			if(pad > 8) {
				const u64 rem = pad - 8;
				memset(dst, 0x66, rem);
				pad -= rem;
				dst += rem;
			}

			std::memcpy(dst, nops[pad - 1], pad);
		}
	}

	void x64_architecture::emit_function_prologue(
		codegen_context& context, utility::byte_buffer& bytecode
	) {
		if(context.stack_usage <= 16) {
			context.prologue_length = 0;
			return;
		}

		// push RBP
		bytecode.append_byte(0x50 + x64::RBP);

		// mov RBP, RSP
		bytecode.append_byte(rex(true, x64::RSP, x64::RBP, 0));
		bytecode.append_byte(0x89);
		bytecode.append_byte(mod_rx_rm(x64::DIRECT, x64::RSP, x64::RBP));

		if(context.stack_usage >= 4096) {
			// emit a chkstk function
			NOT_IMPLEMENTED();
		}
		else {
			bytecode.append_byte(rex(true, 0x00, x64::RSP, 0));

			if (utility::fits_into_8_bits(context.stack_usage)) {
				// sub RSP, stack_usage
				bytecode.append_byte(0x83);
				bytecode.append_byte(mod_rx_rm(x64::DIRECT, 0x05, x64::RSP));
				bytecode.append_byte(static_cast<u8>(context.stack_usage));
			}
			else {
				// sub RSP, stack_usage
				bytecode.append_byte(0x81);
				bytecode.append_byte(mod_rx_rm(x64::DIRECT, 0x05, x64::RSP));
				bytecode.append_dword(static_cast<u32>(context.stack_usage));
			}
		}

		context.prologue_length = static_cast<u8>(bytecode.get_size());
	}

	void x64_architecture::emit_function_body(
		codegen_context& context, utility::byte_buffer& bytecode
	) {
		for (handle<instruction> inst = context.first; inst; inst = inst->next_instruction) {
			const u8 in_base = inst->out_count;
			instruction::category cat;

			if(inst->type >= s_instruction_table.size()) {
				cat = instruction::category::BINOP;
			}
			else {
				cat = s_instruction_table[inst->type].cat;;
			}

			if (
				inst->type == instruction::ENTRY ||
				inst->type == instruction::TERMINATOR
			) { /*does nothing*/ }
			else if(inst->type == instruction::instruction_type::LABEL) {
				const handle<node> basic_block = inst->get<handle<node>>();
				const u64 position = bytecode.get_size();
				const u64 id = context.graph.blocks.at(basic_block).id;

				bytecode.resolve_relocation_dword(&context.labels[id], position);
			}
			else if(inst->type == instruction::INLINE) {
				NOT_IMPLEMENTED();
			}
			else if (inst->type == instruction::EPILOGUE) { /*does nothing*/ }
			else if (inst->type == instruction::LINE) {
				NOT_IMPLEMENTED();
			}
			else if (
				cat == instruction::category::BYTE ||
				cat == instruction::category::BYTE_EXT
			) {
				if(inst->type & instruction::rep) {
					bytecode.append_byte(0xF3);
				}

				emit_instruction_0(inst->type, inst->dt, bytecode);
			}
			else if (inst->type == instruction::ZERO) {
				const handle<codegen_temporary> destination = context.create_temporary();
				const bool is_xmm = inst->dt >= x64::PBYTE && inst->dt <= x64::XMMWORD;

				resolve_interval(context, inst, 0, destination);

				emit_instruction_2(
					context, 
					is_xmm ? instruction::FP_XOR : instruction::XOR, 
					destination, 
					destination,
					inst->dt,
					bytecode
				);
			}
			else if (
				inst->type >= instruction::JMP &&
				inst->type <= instruction::JG
			) {
				handle<codegen_temporary> target;

				if (inst->flags & instruction::node_f) {
					target = codegen_temporary::create_label(context, inst->get<label>().value);
				}
				else if (inst->flags & instruction::global) {
					NOT_IMPLEMENTED();
				}
				else {
					ASSERT(inst->in_count == 1, "");
					resolve_interval(context, inst, in_base, target);
				}

				emit_instruction_1(context, inst->type, target, inst->dt, bytecode);
			}
			else if (inst->type == instruction::CALL) {
				const handle<codegen_temporary> target = context.create_temporary<handle<symbol>>();
				resolve_interval(context, inst, in_base, target);
				emit_instruction_1(context, instruction::CALL, target, x64::QWORD, bytecode);
			}
			else {
				i32 mov_op;
				if(inst->dt >= x64::PBYTE && inst->dt <= x64::XMMWORD) {
					mov_op = instruction::FP_MOV;
				}
				else {
					mov_op = instruction::MOV;
				}

				// prefix
				if (inst->flags & instruction::lock) {
					bytecode.append_byte(0xF0);
				}

				if (inst->flags & instruction::rep) {
					bytecode.append_byte(0xF3);
				}

				i32 dt = inst->dt;
				if (dt == x64::XMMWORD) {
					dt = x64::SSE_PD;
				}

				// resolve output
				handle<codegen_temporary> out = context.create_temporary();
				bool ternary = false;
				u8 i = 0;

				if (inst->out_count == 1) {
					i += resolve_interval(context, inst, i, out);
					ASSERT(i == in_base, "invalid instruction base");
				}
				else {
					i = in_base;
				}

				if (inst->in_count > 0) {
					const handle<codegen_temporary> left = context.create_temporary();
					i += resolve_interval(context, inst, i, left);
					ternary = 
						(i < in_base + inst->in_count) ||
						(inst->flags & (instruction::immediate | instruction::absolute));

					if (
						ternary &&
						inst->type == instruction::IMUL &&
						(inst->flags & instruction::immediate)
					) {
						// there's a special case for ternary IMUL r64, r/m64, imm32
						emit_instruction_2(context, instruction::IMUL3, out, left, dt, bytecode);

						if(inst->dt == x64::WORD) {
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
					else if (inst->type == instruction::IDIV || inst->type == instruction::DIV) {
						emit_instruction_1(context, inst->type, left, dt, bytecode);
						continue;
					}
					else {
						if (out == nullptr) {
							out = context.create_temporary();
						}

						if (ternary || inst->type == instruction::MOV || inst->type == instruction::FP_MOV) {
							if (out->matches(left) == false) {
								emit_instruction_2(
									context, 
									static_cast<instruction::instruction_type>(mov_op), 
									out, 
									left,
									dt, 
									bytecode
								);
							}
						}
						else {
							emit_instruction_2(context, inst->type, out, left, dt, bytecode);
						}
					}
				}

				// unary ops
				if (cat == instruction::category::UNARY || cat == instruction::category::UNARY_EXT) {
					emit_instruction_1(context, inst->type, out, dt, bytecode);
					continue;
				}

				if (inst->flags & instruction::immediate) {
					const handle<codegen_temporary> right = codegen_temporary::create_imm(
						context, inst->get<immediate>().value
					);

					emit_instruction_2(context, inst->type, out, right, dt, bytecode);
				}
				else if (inst->flags & instruction::absolute) {
					const handle<codegen_temporary> right = codegen_temporary::create_abs(
						context, inst->get<immediate>().value
					);

					emit_instruction_2(context, inst->type, out, right, dt, bytecode);
				}
				else if (ternary) {
					const handle<codegen_temporary> right = context.create_temporary();
					resolve_interval(context, inst, i, right);

					if (
						inst->type != instruction::MOV || 
						(inst->type == instruction::MOV && out->matches(right) == false)
					) {
						emit_instruction_2(context, inst->type, out, right, dt, bytecode);
					}
				}
			}
		}
	}

	void x64_architecture::emit_function_epilogue(
		const codegen_context& context, utility::byte_buffer& bytecode
	) {
		ASSERT(context.func->exit_node != nullptr, "no exit node found");

		if(context.stack_usage <= 16) {
			bytecode.append_byte(0xC3);
			return;
		}

		// add RSP, stack_usage
		bytecode.append_byte(rex(true, 0x00, x64::RSP, 0));

		if (utility::fits_into_8_bits(context.stack_usage)) {
			bytecode.append_byte(0x83);
			bytecode.append_byte(mod_rx_rm(x64::DIRECT, 0x00, x64::RSP));
			bytecode.append_byte(static_cast<i8>(context.stack_usage));
		}
		else {
			bytecode.append_byte(0x81);
			bytecode.append_byte(mod_rx_rm(x64::DIRECT, 0x00, x64::RSP));
			bytecode.append_dword(static_cast<u32>(context.stack_usage));
		}

		// pop RBP
		bytecode.append_byte(0x58 + x64::RBP);

		// ret
		const handle<node> remote_procedure_call = context.func->exit_node->inputs[2];

		if(
			remote_procedure_call->ty == node::PROJECTION &&
			remote_procedure_call->inputs[0]->ty == node::ENTRY &&
			remote_procedure_call->get<projection>().index == 2
		) {
			bytecode.append_byte(0xC3);
		}
	}

	auto x64_architecture::get_instruction_table() ->std::array<instruction::description, 120> {
		std::array<instruction::description, 120> table = {};

		table[instruction::RET        ] = { .mnemonic = "ret",       .cat = instruction::category::BYTE,     .op = 0xC3 };
		table[instruction::INT3       ] = { .mnemonic = "int3",      .cat = instruction::category::BYTE,     .op = 0xCC };
		table[instruction::STOSB      ] = { .mnemonic = "rep stosb", .cat = instruction::category::BYTE,     .op = 0xAA };
		table[instruction::MOVSB      ] = { .mnemonic = "rep movsb", .cat = instruction::category::BYTE,     .op = 0xA4 };
		table[instruction::CAST       ] = { .mnemonic = "cvt",       .cat = instruction::category::BYTE,     .op = 0x99 };
		table[instruction::SYS_CALL   ] = { .mnemonic = "syscall",   .cat = instruction::category::BYTE_EXT, .op = 0x05 };
		table[instruction::RDTSC      ] = { .mnemonic = "rdtsc",     .cat = instruction::category::BYTE_EXT, .op = 0x31 };
		table[instruction::UD2        ] = { .mnemonic = "ud2",       .cat = instruction::category::BYTE_EXT, .op = 0x0B };

		table[instruction::NOT         ] = { .mnemonic = "not",  .cat = instruction::category::UNARY, .op = 0xF7, .rx_i = 0x02 };
		table[instruction::NEG         ] = { .mnemonic = "neg",  .cat = instruction::category::UNARY, .op = 0xF7, .rx_i = 0x03 };
		table[instruction::MUL         ] = { .mnemonic = "mul",  .cat = instruction::category::UNARY, .op = 0xF7, .rx_i = 0x04 };
		table[instruction::DIV         ] = { .mnemonic = "div",  .cat = instruction::category::UNARY, .op = 0xF7, .rx_i = 0x06 };
		table[instruction::IDIV        ] = { .mnemonic = "idiv", .cat = instruction::category::UNARY, .op = 0xF7, .rx_i = 0x07 };
		table[instruction::CALL        ] = { .mnemonic = "call", .cat = instruction::category::UNARY, .op = 0xE8, .rx_i = 0x02 };
		table[instruction::JMP         ] = { .mnemonic = "jmp",  .cat = instruction::category::UNARY, .op = 0xE9, .rx_i = 0x04 };

		// prefetching
		table[instruction::PREFETCHNTA] = { .mnemonic = "prefetchnta", .cat = instruction::category::UNARY_EXT, .op = 0x18, .rx_i = 0 };
		table[instruction::PREFETCH0  ] = { .mnemonic = "prefetch0",   .cat = instruction::category::UNARY_EXT, .op = 0x18, .rx_i = 1 };
		table[instruction::PREFETCH1  ] = { .mnemonic = "prefetch1",   .cat = instruction::category::UNARY_EXT, .op = 0x18, .rx_i = 2 };
		table[instruction::PREFETCH2  ] = { .mnemonic = "prefetch2",   .cat = instruction::category::UNARY_EXT, .op = 0x18, .rx_i = 3 };

		// jcc
		table[instruction::JO ] = { .mnemonic = "jo",  .cat = instruction::category::UNARY_EXT, .op = 0x80 };
		table[instruction::JNO] = { .mnemonic = "jno", .cat = instruction::category::UNARY_EXT, .op = 0x81 };
		table[instruction::JB] =  { .mnemonic = "jb",  .cat = instruction::category::UNARY_EXT, .op = 0x82 };
		table[instruction::JNB] = { .mnemonic = "jnb", .cat = instruction::category::UNARY_EXT, .op = 0x83 };
		table[instruction::JE] =  { .mnemonic = "je",  .cat = instruction::category::UNARY_EXT, .op = 0x84 };
		table[instruction::JNE] = { .mnemonic = "jne", .cat = instruction::category::UNARY_EXT, .op = 0x85 };
		table[instruction::JBE] = { .mnemonic = "jbe", .cat = instruction::category::UNARY_EXT, .op = 0x86 };
		table[instruction::JA] =  { .mnemonic = "ja",  .cat = instruction::category::UNARY_EXT, .op = 0x87 };
		table[instruction::JS] =  { .mnemonic = "js",  .cat = instruction::category::UNARY_EXT, .op = 0x88 };
		table[instruction::JNS] = { .mnemonic = "jns", .cat = instruction::category::UNARY_EXT, .op = 0x89 };
		table[instruction::JP] =  { .mnemonic = "jp",  .cat = instruction::category::UNARY_EXT, .op = 0x8A };
		table[instruction::JNP] = { .mnemonic = "jnp", .cat = instruction::category::UNARY_EXT, .op = 0x8B };
		table[instruction::JL] =  { .mnemonic = "jl",  .cat = instruction::category::UNARY_EXT, .op = 0x8C };
		table[instruction::JGE] = { .mnemonic = "jge", .cat = instruction::category::UNARY_EXT, .op = 0x8D };
		table[instruction::JLE] = { .mnemonic = "jle", .cat = instruction::category::UNARY_EXT, .op = 0x8E };
		table[instruction::JG] =  { .mnemonic = "jg",  .cat = instruction::category::UNARY_EXT, .op = 0x8F };

		// setcc
		table[instruction::SETO ] = { .mnemonic = "seto",  .cat = instruction::category::UNARY_EXT, .op = 0x90 };
		table[instruction::SETNO] = { .mnemonic = "setno", .cat = instruction::category::UNARY_EXT, .op = 0x91 };
		table[instruction::SETB ] = { .mnemonic = "setb",  .cat = instruction::category::UNARY_EXT, .op = 0x92 };
		table[instruction::SETNB] = { .mnemonic = "setnb", .cat = instruction::category::UNARY_EXT, .op = 0x93 };
		table[instruction::SETE ] = { .mnemonic = "sete",  .cat = instruction::category::UNARY_EXT, .op = 0x94 };
		table[instruction::SETNE] = { .mnemonic = "setne", .cat = instruction::category::UNARY_EXT, .op = 0x95 };
		table[instruction::SETBE] = { .mnemonic = "setbe", .cat = instruction::category::UNARY_EXT, .op = 0x96 };
		table[instruction::SETA ] = { .mnemonic = "seta",  .cat = instruction::category::UNARY_EXT, .op = 0x97 };
		table[instruction::SETS ] = { .mnemonic = "sets",  .cat = instruction::category::UNARY_EXT, .op = 0x98 };
		table[instruction::SETNS] = { .mnemonic = "setns", .cat = instruction::category::UNARY_EXT, .op = 0x99 };
		table[instruction::SETP ] = { .mnemonic = "setp",  .cat = instruction::category::UNARY_EXT, .op = 0x9A };
		table[instruction::SETNP] = { .mnemonic = "setnp", .cat = instruction::category::UNARY_EXT, .op = 0x9B };
		table[instruction::SETL ] = { .mnemonic = "setl",  .cat = instruction::category::UNARY_EXT, .op = 0x9C };
		table[instruction::SETGE] = { .mnemonic = "setge", .cat = instruction::category::UNARY_EXT, .op = 0x9D };
		table[instruction::SETLE] = { .mnemonic = "setle", .cat = instruction::category::UNARY_EXT, .op = 0x9E };
		table[instruction::SETG ] = { .mnemonic = "setg",  .cat = instruction::category::UNARY_EXT, .op = 0x9F };

		// cmovcc
		table[instruction::CMOVO ] = { .mnemonic = "cmovo",  .cat = instruction::category::BINOP_EXT_1, .op = 0x40 };
		table[instruction::CMOVNO] = { .mnemonic = "cmovno", .cat = instruction::category::BINOP_EXT_1, .op = 0x41 };
		table[instruction::CMOVB ] = { .mnemonic = "cmovb",  .cat = instruction::category::BINOP_EXT_1, .op = 0x42 };
		table[instruction::CMOVNB] = { .mnemonic = "cmovnb", .cat = instruction::category::BINOP_EXT_1, .op = 0x43 };
		table[instruction::CMOVE ] = { .mnemonic = "cmove",  .cat = instruction::category::BINOP_EXT_1, .op = 0x44 };
		table[instruction::CMOVNE] = { .mnemonic = "cmovne", .cat = instruction::category::BINOP_EXT_1, .op = 0x45 };
		table[instruction::CMOVBE] = { .mnemonic = "cmovbe", .cat = instruction::category::BINOP_EXT_1, .op = 0x46 };
		table[instruction::CMOVA ] = { .mnemonic = "cmova",  .cat = instruction::category::BINOP_EXT_1, .op = 0x47 };
		table[instruction::CMOVS ] = { .mnemonic = "cmovs",  .cat = instruction::category::BINOP_EXT_1, .op = 0x48 };
		table[instruction::CMOVNS] = { .mnemonic = "cmovns", .cat = instruction::category::BINOP_EXT_1, .op = 0x49 };
		table[instruction::CMOVP ] = { .mnemonic = "cmovp",  .cat = instruction::category::BINOP_EXT_1, .op = 0x4A };
		table[instruction::CMOVNP] = { .mnemonic = "cmovnp", .cat = instruction::category::BINOP_EXT_1, .op = 0x4B };
		table[instruction::CMOVL ] = { .mnemonic = "cmovl",  .cat = instruction::category::BINOP_EXT_1, .op = 0x4C };
		table[instruction::CMOVGE] = { .mnemonic = "cmovge", .cat = instruction::category::BINOP_EXT_1, .op = 0x4D };
		table[instruction::CMOVLE] = { .mnemonic = "cmovle", .cat = instruction::category::BINOP_EXT_1, .op = 0x4E };
		table[instruction::CMOVG ] = { .mnemonic = "cmovg",  .cat = instruction::category::BINOP_EXT_1, .op = 0x4F };

		// bit magic
		table[instruction::BSF] = { .mnemonic = "bsf", .cat = instruction::category::BINOP_EXT_1, .op = 0xBC };
		table[instruction::BSF] = { .mnemonic = "bsr", .cat = instruction::category::BINOP_EXT_1, .op = 0xBD };

		// binary ops but they have an implicit CL on the right-hand side
		table[instruction::SHL] = { .mnemonic = "shl", .cat = instruction::category::BINOP_CL, .op = 0xD2, .op_i = 0xC0, .rx_i = 0x04 };
		table[instruction::SHR] = { .mnemonic = "shr", .cat = instruction::category::BINOP_CL, .op = 0xD2, .op_i = 0xC0, .rx_i = 0x05 };
		table[instruction::ROL] = { .mnemonic = "rol", .cat = instruction::category::BINOP_CL, .op = 0xD2, .op_i = 0xC0, .rx_i = 0x00 };
		table[instruction::ROR] = { .mnemonic = "ror", .cat = instruction::category::BINOP_CL, .op = 0xD2, .op_i = 0xC0, .rx_i = 0x01 };
		table[instruction::SAR] = { .mnemonic = "sar", .cat = instruction::category::BINOP_CL, .op = 0xD2, .op_i = 0xC0, .rx_i = 0x07 };

		table[instruction::ADD ] =  { .mnemonic = "add",  .cat = instruction::category::BINOP, .op = 0x00, .op_i = 0x80, .rx_i = 0x00 };
		table[instruction::OR  ] =  { .mnemonic = "or",   .cat = instruction::category::BINOP, .op = 0x08, .op_i = 0x80, .rx_i = 0x01 };
		table[instruction::AND ] =  { .mnemonic = "and",  .cat = instruction::category::BINOP, .op = 0x20, .op_i = 0x80, .rx_i = 0x04 };
		table[instruction::SUB ] =  { .mnemonic = "sub",  .cat = instruction::category::BINOP, .op = 0x28, .op_i = 0x80, .rx_i = 0x05 };
		table[instruction::XOR ] =  { .mnemonic = "xor",  .cat = instruction::category::BINOP, .op = 0x30, .op_i = 0x80, .rx_i = 0x06 };
		table[instruction::CMP ] =  { .mnemonic = "cmp",  .cat = instruction::category::BINOP, .op = 0x38, .op_i = 0x80, .rx_i = 0x07 };
		table[instruction::MOV ] =  { .mnemonic = "mov",  .cat = instruction::category::BINOP, .op = 0x88, .op_i = 0xC6, .rx_i = 0x00 };
		table[instruction::TEST] =  { .mnemonic = "test", .cat = instruction::category::BINOP, .op = 0x84, .op_i = 0xF6, .rx_i = 0x00 };

		// misc integer ops
		table[instruction::MOVABS                 ] =  { .mnemonic = "mov",    .cat = instruction::category::BINOP_PLUS, .op = 0xB8 };
		table[instruction::XCHG                   ] =  { .mnemonic = "xchg",   .cat = instruction::category::BINOP,      .op = 0x86 };
		table[instruction::LEA                    ] =  { .mnemonic = "lea",    .cat = instruction::category::BINOP,      .op = 0x8D };
		table[instruction::XADD                   ] =  { .mnemonic = "xadd",   .cat = instruction::category::BINOP_EXT_1,  .op = 0xC0 };
		table[instruction::IMUL                   ] =  { .mnemonic = "imul",   .cat = instruction::category::BINOP_EXT_1,  .op = 0xAF };
		table[instruction::IMUL3                  ] =  { .mnemonic = "imul",   .cat = instruction::category::BINOP,      .op = 0x69 };
		table[instruction::MOVSXB                 ] =  { .mnemonic = "movsxb", .cat = instruction::category::BINOP_EXT_2, .op = 0xBE };
		table[instruction::MOVSXW                 ] =  { .mnemonic = "movsxw", .cat = instruction::category::BINOP_EXT_2, .op = 0xBF };
		table[instruction::MOVSXD                 ] =  { .mnemonic = "movsxd", .cat = instruction::category::BINOP,      .op = 0x63 };
		table[instruction::MOVZXB                 ] =  { .mnemonic = "movzxb", .cat = instruction::category::BINOP_EXT_2, .op = 0xB6 };
		table[instruction::MOVZXW                 ] =  { .mnemonic = "movzxw", .cat = instruction::category::BINOP_EXT_2, .op = 0xB7 };

		// gpr<->xmm
		table[instruction::MOV_I2F] = { .mnemonic = "mov", .cat = instruction::category::BINOP_EXT_3, .op = 0x6E };
		table[instruction::MOV_F2I] = { .mnemonic = "mov", .cat = instruction::category::BINOP_EXT_3, .op = 0x7E };

		// SSE binary operations
		table[instruction::FP_MOV            ] = {  .mnemonic ="mov",   .cat = instruction::category::BINOP_SSE, .op = 0x10 };
		table[instruction::FP_ADD            ] = {  .mnemonic ="add",   .cat = instruction::category::BINOP_SSE, .op = 0x58 };
		table[instruction::FP_MUL            ] = {  .mnemonic ="mul",   .cat = instruction::category::BINOP_SSE, .op = 0x59 };
		table[instruction::FP_SUB            ] = {  .mnemonic ="sub",   .cat = instruction::category::BINOP_SSE, .op = 0x5C };
		table[instruction::FP_MIN            ] = {  .mnemonic ="min",   .cat = instruction::category::BINOP_SSE, .op = 0x5D };
		table[instruction::FP_DIV            ] = {  .mnemonic ="div",   .cat = instruction::category::BINOP_SSE, .op = 0x5E };
		table[instruction::FP_MAX            ] = {  .mnemonic ="max",   .cat = instruction::category::BINOP_SSE, .op = 0x5F };
		table[instruction::FP_CMP            ] = {  .mnemonic ="cmp",   .cat = instruction::category::BINOP_SSE, .op = 0xC2 };
		table[instruction::FP_UCOMI          ] = {  .mnemonic ="ucomi", .cat = instruction::category::BINOP_SSE, .op = 0x2E };
		table[instruction::FP_CVT32          ] = {  .mnemonic ="cvtsi", .cat = instruction::category::BINOP_SSE, .op = 0x2A };
		table[instruction::FP_CVT64          ] = {  .mnemonic ="cvtsi", .cat = instruction::category::BINOP_SSE, .op = 0x2A };
		table[instruction::FP_CVT            ] = {  .mnemonic ="cvt",   .cat = instruction::category::BINOP_SSE, .op = 0x5A };
		table[instruction::FP_CVTT           ] = {  .mnemonic ="rsqrt", .cat = instruction::category::BINOP_SSE, .op = 0x2C };
		table[instruction::FP_SQRT           ] = {  .mnemonic ="and",   .cat = instruction::category::BINOP_SSE, .op = 0x51 };
		table[instruction::FP_RSQRT          ] = {  .mnemonic ="or",    .cat = instruction::category::BINOP_SSE, .op = 0x52 };
		table[instruction::FP_AND            ] = {  .mnemonic ="xor",   .cat = instruction::category::BINOP_SSE, .op = 0x54 };
		table[instruction::FP_OR             ] = {  .mnemonic ="or",    .cat = instruction::category::BINOP_SSE, .op = 0x56 };
		table[instruction::FP_XOR            ] = {  .mnemonic ="xor",   .cat = instruction::category::BINOP_SSE, .op = 0x57 };

		return table;
	}

	void x64_architecture::emit_instruction_0(
		instruction::instruction_type type, i32 data_type, utility::byte_buffer& bytecode
	) {
		ASSERT(type < s_instruction_table.size(), "type is out of range");
		const auto& description = &s_instruction_table[type];

		if (data_type == x64::QWORD) {
			bytecode.append_byte(0x48);
		}

		if (description->cat == instruction::category::BYTE_EXT) {
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

	void x64_architecture::emit_instruction_1(
		codegen_context& context,
		instruction::instruction_type type,
		handle<codegen_temporary> r,
		i32 dt, 
		utility::byte_buffer& bytecode
	) {
		ASSERT(type < s_instruction_table.size(), "invalid type");
		const instruction::description& descriptor = s_instruction_table[type];

		const bool is_rex = dt == x64::BYTE || dt == x64::QWORD;
		const bool is_rexw = dt == x64::QWORD;
		const u8 op = descriptor.op_i;
		const u8 rx = descriptor.rx_i;

		if (r->type == codegen_temporary::GPR) {
			if (is_rex || r->reg >= 8) {
				bytecode.append_byte(rex(is_rexw, 0x00, r->reg, 0x00));
			}

			if (descriptor.cat == instruction::category::UNARY_EXT) {
				bytecode.append_byte(0x0F);
			}

			bytecode.append_byte(op ? op : descriptor.op);
			bytecode.append_byte(mod_rx_rm(x64::DIRECT, rx, r->reg));
		}
		else if (r->type == codegen_temporary::MEM) {
			const i32 displacement = r->IMM;
			const u8 index = r->index;
			const u8 base = r->reg;
			scale s = r->sc;

			const bool needs_index = (index != reg::invalid_id) || (base & 7) == x64::RSP;
			bytecode.append_byte(rex(is_rexw, 0x00, base, index != reg::invalid_id ? index : 0));

			if (descriptor.cat == instruction::category::UNARY_EXT) {
				bytecode.append_byte(0x0F);
			}

			bytecode.append_byte(op);

			// if it needs an index, it'll put RSP into the base slot
			// and write the real base into the SIB
			x64::mod m = x64::INDIRECT_DISPLACEMENT_32;

			if (utility::fits_into_8_bits(displacement)) {
				m = x64::INDIRECT_DISPLACEMENT_8;
			}

			bytecode.append_byte(mod_rx_rm(m, rx, needs_index ? x64::RSP : base));

			if (needs_index) {
				bytecode.append_byte(
					mod_rx_rm(static_cast<x64::mod>(s), (base & 7) == x64::RSP ? x64::RSP : index, base)
				);
			}

			if (m == x64::INDIRECT_DISPLACEMENT_8) {
				bytecode.append_byte(static_cast<i8>(displacement));
			}
			else if (m == x64::INDIRECT_DISPLACEMENT_32) {
				bytecode.append_dword(static_cast<i32>(displacement));
			}
		}
		else if (r->type == codegen_temporary::GLOBAL) {
			if(descriptor.op) {
				if(descriptor.cat == instruction::category::UNARY_EXT) {
					bytecode.append_byte(0x0F);
				}

				bytecode.append_byte(descriptor.op);
			}
			else {
				if(is_rex) {
					bytecode.append_byte(is_rexw ? 0x48 : 0x40);
				}

				if (descriptor.cat == instruction::category::UNARY_EXT) {
					bytecode.append_byte(0x0F);
				}

				bytecode.append_byte(op);
				bytecode.append_byte(((rx & 7) << 3) | x64::RBP);
			}

			bytecode.append_dword(r->immediate);
			emit_symbol_patch(context, r->get<handle<symbol>>(), bytecode.get_size() - 4);
		}
		else if (r->type == codegen_temporary::LABEL) {
			if (descriptor.cat == instruction::category::UNARY_EXT) {
				bytecode.append_byte(0x0F);
			}

			bytecode.append_byte(descriptor.op);
			bytecode.append_dword(0);

			const u64 label = r->get<sigma::ir::label>().value;
			bytecode.emit_relocation_dword(&context.labels[label], bytecode.get_size() - 4);
		}
		else {
			NOT_IMPLEMENTED();
		}
	}

	void x64_architecture::emit_instruction_2(
		codegen_context& context,
		instruction::instruction_type type,
		handle<codegen_temporary> a,
		handle<codegen_temporary> b, 
		i32 data_type,
		utility::byte_buffer& bytecode
	) {
		if (data_type >= x64::SSE_SS && data_type <= x64::SSE_PD) {
			NOT_IMPLEMENTED();
		}

		ASSERT(data_type >= x64::BYTE && data_type <= x64::QWORD, "invalid data type");
		ASSERT(type < s_instruction_table.size(), "invalid type");

		const instruction::description& descriptor = s_instruction_table[type];
		const bool dir = b->type == codegen_temporary::MEM || b->type == codegen_temporary::GLOBAL;

		if (type == instruction::MOVABS) {
			ASSERT(
				a->type == codegen_temporary::GPR && b->type == codegen_temporary::ABS,
				"invalid data types for a movabs operation"
			);

			bytecode.append_byte(rex(true, a->reg, 0, 0));
			bytecode.append_byte(descriptor.op + (a->reg & 0b111));
			bytecode.append_qword(b->get<absolute>().value);
		}

		if (
			dir ||
			descriptor.op == 0x63 ||
			descriptor.op == 0x69 ||
			descriptor.op == 0x6E ||
			(type >= instruction::CMOVO && type <= instruction::CMOVG) ||
			descriptor.op == 0xAF ||
			descriptor.cat == instruction::category::BINOP_EXT_2
		) {
			std::swap(a, b);
		}

		// operand size 
		bool sz = data_type != x64::BYTE;

		// uses an imm value that works as a sign extended 8 bit number
		const bool short_imm =
			sz && b->type == codegen_temporary::IMM &&
			b->immediate == static_cast<i8>(b->immediate) &&
			descriptor.op_i == 0x80;

		// the destination can only be a GPR, no direction flag
		const bool is_gpr_only_dst = descriptor.op & 1;
		const bool dir_flag = dir != is_gpr_only_dst && descriptor.op != 0x69;

		if (descriptor.cat != instruction::category::BINOP_EXT_3) {
			// address size prefix
			if (data_type == x64::WORD && descriptor.cat != instruction::category::BINOP_EXT_2) {
				bytecode.append_byte(0x66);
			}

			ASSERT(
				b->type == codegen_temporary::GPR || b->type == codegen_temporary::IMM,
				"secondary operand is invalid!"
			);
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

		if (a->type == codegen_temporary::MEM || a->type == codegen_temporary::GPR) {
			base = a->reg;
		}
		else {
			base = x64::RBP;
		}

		if (a->type == codegen_temporary::MEM && a->index != reg::invalid_id) {
			rex_prefix |= ((a->index >> 3) << 1);
		}

		u8 rx;
		if(b->type == codegen_temporary::GPR || b->type == codegen_temporary::XMM) {
			rx = b->reg;
		}
		else {
			rx = descriptor.rx_i;
		}

		if (descriptor.cat == instruction::category::BINOP_CL) {
			ASSERT(
				b->type == codegen_temporary::IMM || 
				(b->type == codegen_temporary::GPR && b->reg == x64::RCX),
				"invalid binary operation"
			);

			data_type = x64::BYTE;
			rx = descriptor.rx_i;
		}

		rex_prefix |= (base >> 3);
		rex_prefix |= (rx >> 3) << 2;

		// if the REX stays as 0x40 then it's default and doesn't need to be here
		if (rex_prefix != 0x40 || data_type == x64::BYTE || type == instruction::MOVZXB) {
			bytecode.append_byte(rex_prefix);
		}

		if (descriptor.cat == instruction::category::BINOP_EXT_3) {
			bytecode.append_byte(0x0F);
			bytecode.append_byte(descriptor.op);
		}
		else {
			// opcode
			if (
				descriptor.cat == instruction::category::BINOP_EXT_1 || 
				descriptor.cat == instruction::category::BINOP_EXT_2
			) {
				// DEF instructions can only be 32bit and 64bit... maybe?
				if (type != instruction::XADD) {
					sz = false;
				}

				bytecode.append_byte(0x0F);
			}

			// immediates have a custom opcode
			ASSERT(
				b->type != codegen_temporary::IMM || descriptor.op_i != 0 || descriptor.rx_i != 0, 
				"no immediate variant of instruction"
			);

			u8 opcode = b->type == codegen_temporary::IMM ? descriptor.op_i : descriptor.op;

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

		if (b->type == codegen_temporary::IMM) {
			if (data_type == x64::BYTE || short_imm) {
				if (short_imm) {
					ASSERT(
						b->immediate == static_cast<i8>(b->immediate), 
						"invalid short immediate"
					);
				}

				bytecode.append_byte(static_cast<i8>(b->immediate));
			}
			else if (data_type == x64::WORD) {
				const i32 imm = b->immediate;
				ASSERT(
					(imm & 0xFFFF0000) == 0xFFFF0000 || (imm & 0xFFFF0000) == 0,
					"invalid immediate"
				);

				bytecode.append_word(static_cast<u16>(imm));
			}
			else {
				bytecode.append_dword(b->immediate);
			}
		}

		if (a->type == codegen_temporary::GLOBAL && 
			disp_patch + 4 != bytecode.get_size()
		) {
			bytecode.patch_dword(
				disp_patch, static_cast<u32>(disp_patch + 4 - bytecode.get_size())
			);
		}
	}

	void x64_architecture::emit_memory_operand(
		codegen_context& context, u8 rx, handle<codegen_temporary> a, utility::byte_buffer& bytecode
	) {
		// operand encoding
		if (a->type == codegen_temporary::GPR || a->type == codegen_temporary::XMM) {
			bytecode.append_byte(mod_rx_rm(x64::DIRECT, rx, a->reg));
		}
		else if (a->type == codegen_temporary::MEM) {
			const u8 base = a->reg;
			const u8 index = a->index;
			scale scale = a->sc;
			const i32 displacement = a->immediate;
			const bool needs_index = (index != reg::invalid_id) || (base & 7) == x64::RSP;
			
			// if it needs an index, it'll put RSP into the base slot and write the real base
			// into the SIB
			x64::mod m = x64::INDIRECT_DISPLACEMENT_32;
			
			if (displacement == 0 && (base & 7) != x64::RBP) {
				m = x64::INDIRECT;
			}
			else if (displacement == static_cast<i8>(displacement)) {
				m = x64::INDIRECT_DISPLACEMENT_8;
			}

			bytecode.append_byte(mod_rx_rm(m, rx, needs_index ? x64::RSP : base));
			
			if (needs_index) {
				bytecode.append_byte(
					mod_rx_rm(static_cast<x64::mod>(scale), (base & 7) == x64::RSP ? x64::RSP : index, base)
				);
			}
			
			if (m == x64::INDIRECT_DISPLACEMENT_8) {
				bytecode.append_byte(static_cast<i8>(displacement));
			}
			else if (m == x64::INDIRECT_DISPLACEMENT_32) {
				bytecode.append_dword(displacement);
			}
		}
		else if (a->type == codegen_temporary::GLOBAL) {
			bytecode.append_byte(((rx & 7) << 3) | x64::RBP);
			bytecode.append_dword(a->immediate);
			emit_symbol_patch(context, a->get<handle<symbol>>(), bytecode.get_size() - 4);
		}
		else {
			NOT_IMPLEMENTED();
		}
	}

	auto x64_architecture::resolve_interval(
		const codegen_context& context,
		handle<instruction> inst,
		u8 i,
		handle<codegen_temporary> val
	) -> u8 {
		handle interval = &context.intervals[inst->operands[i]];

		if (
			(inst->flags & (instruction::mem_f |instruction::global)) &&
			i == inst->memory_slot
		) {
			ASSERT(interval->spill <= 0, "cannot use spilled value for a memory operand");

			if (inst->flags & instruction::mem_f) {
				val->type = codegen_temporary::MEM;
				val->reg = interval->assigned.id;
				val->index = reg::invalid_id;
				val->sc = inst->sc;
				val->immediate = inst->displacement;

				if (inst->flags & instruction::indexed) {
					interval = &context.intervals[inst->operands[i + 1]];
					ASSERT(interval->spill <= 0, "cannot use spilled value for a memory operand");

					val->index = interval->assigned.id;
					return 2;
				}

				return 1;
			}

			val->type = codegen_temporary::GLOBAL;
			val->immediate = inst->displacement;
			val->get<handle<symbol>>() = inst->get<handle<symbol>>();

			return 1;
		}

		if (interval->spill > 0) {
			val->type = codegen_temporary::MEM;
			val->reg = x64::RBP;
			val->index = reg::invalid_id;
			val->immediate = -interval->spill;
		}
		else {
			if(interval->r.cl == x64::register_class::XMM) {
				val->type = codegen_temporary::XMM;
			}
			else {
				val->type = codegen_temporary::GPR;
			}

			val->reg = interval->assigned.id;
		}

		return 1;
	}

	void x64_architecture::emit_symbol_patch(
		codegen_context& context, handle<symbol> target, u64 pos
	) {
		const handle<symbol_patch> patch = context.create_symbol_patch();

		patch->target = target;
		patch->next = nullptr;
		patch->pos = pos;

		context.patch_count++;

		if(context.first_patch == nullptr) {
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
