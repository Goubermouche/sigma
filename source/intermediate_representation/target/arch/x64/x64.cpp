#include "x64.h"

#include "intermediate_representation/codegen/instruction.h"

namespace ir {
	utility::byte_buffer x64_architecture::emit_bytecode(codegen_context& context) {
		resolve_stack_usage(context);

		utility::byte_buffer bytecode;

		// generate the bytecode for our function
		emit_function_prologue(context, bytecode);
		emit_function_body(context, bytecode);
		emit_function_epilogue(context, bytecode);

		return bytecode;
	}

	std::vector<live_interval> x64_architecture::get_register_intervals() {
		std::vector<live_interval> intervals(32);

		for (u8 i = 0; i < 32; ++i) {
			const bool is_gpr = i < 16;

			classified_reg reg;
			reg.id = i % 16;
			reg.cl = is_gpr ? x64::register_class::gpr : x64::register_class::xmm;

			intervals[i] = live_interval{
				.reg = reg,
				.data_type = is_gpr ? x64::qword : x64::xmmword,
				.ranges = { utility::range<u64>::max() }
			};
		}

		return intervals;
	}

	void x64_architecture::resolve_stack_usage(codegen_context& context) {
		u64 caller_usage = context.caller_usage;

		if(context.target.get_abi() == abi::win64 && caller_usage > 0 && caller_usage < 4) {
			caller_usage = 4;
		}

		const u64 usage = context.stack_usage + caller_usage * 8;
		context.stack_usage = utility::align(usage, 16);
	}

	void x64_architecture::emit_function_prologue(
		codegen_context& context, utility::byte_buffer& bytecode
	) {
		if(context.stack_usage <= 16) {
			context.prologue_length = 0;
			return;
		}

		// push RBP
		bytecode.append_byte(0x50 + x64::rbp);

		// mov RBP, RSP
		bytecode.append_byte(rex(true, x64::rsp, x64::rbp, 0));
		bytecode.append_byte(0x89);
		bytecode.append_byte(mod_rx_rm(x64::direct, x64::rsp, x64::rbp));

		if(context.stack_usage >= 4096) {
			// emit a chkstk function
		}
		else {
			bytecode.append_byte(rex(true, 0x00, x64::rsp, 0));

			if (utility::fits_into_8_bits(context.stack_usage)) {
				// sub RSP, stack_usage
				bytecode.append_byte(0x83);
				bytecode.append_byte(mod_rx_rm(x64::direct, 0x05, x64::rsp));
				bytecode.append_byte(static_cast<u8>(context.stack_usage));
			}
			else {
				// sub RSP, stack_usage
				bytecode.append_byte(0x81);
				bytecode.append_byte(mod_rx_rm(x64::direct, 0x05, x64::rsp));
				bytecode.append_dword(static_cast<u32>(context.stack_usage));
			}
		}

		context.prologue_length = bytecode.get_size();
	}

	void x64_architecture::emit_function_body(
		codegen_context& context, utility::byte_buffer& bytecode
	) {
		for (handle<instruction> inst = context.first; inst; inst = inst->next_instruction) {
			const u8 in_base = inst->out_count;
			const auto cat = inst->ty >= s_instruction_table.size() ? instruction::category::binop : s_instruction_table[inst->ty].cat;

			if (
				inst->ty == instruction::entry ||
				inst->ty == instruction::terminator
			) { /*does nothing*/ }
			else if(inst->ty == instruction::type::label) {
				const handle<node> basic_block = inst->get<handle<node>>();
				const u64 position = bytecode.get_size();
				const u64 id = context.graph.blocks.at(basic_block).id;
				bytecode.resolve_relocation_dword(&context.labels[id], position);
			}
			else if(inst->ty == instruction::inl) {
				ASSERT(false, "not implemented 3");
			}
			else if (inst->ty == instruction::epilogue) { /*does nothing*/ }
			else if (inst->ty == instruction::line) {
				ASSERT(false, "not implemented 5");
			}
			else if (
				cat == instruction::category::byte ||
				cat == instruction::category::byte_ext
			) {
				if(inst->ty & instruction::rep) {
					bytecode.append_byte(0xF3);
				}

				emit_instruction_0(inst->ty, inst->dt, bytecode);
			}
			else if (inst->ty == instruction::zero) {
				const handle<value> destination = context.create_value();
				const bool is_xmm = inst->dt >= x64::pbyte && inst->dt <= x64::xmmword;

				resolve_interval(context, inst, 0, destination);
				emit_instruction_2(context, is_xmm ? instruction::FP_XOR : instruction::XOR, destination, destination, inst->dt, bytecode);
			}
			else if (
				inst->ty >= instruction::jmp &&
				inst->ty <= instruction::JG
			) {
				handle<value> target;

				if (inst->fl & instruction::node_f) {
					target = value::create_label(context, inst->get<label>().value);
				}
				else if (inst->fl & instruction::global) {
					ASSERT(false, "not implemented");
				}
				else {
					ASSERT(inst->in_count == 1, "");
					resolve_interval(context, inst, in_base, target);
				}

				emit_instruction_1(context, inst->ty, target, inst->dt, bytecode);
			}
			else if (inst->ty == instruction::call) {
				const handle<value> target = context.create_value<handle<symbol>>();
				resolve_interval(context, inst, in_base, target);
				emit_instruction_1(context, instruction::call, target, x64::qword, bytecode);
			}
			else {
				i32 mov_op = inst->dt >= x64::pbyte && inst->dt <= x64::xmmword ? instruction::FP_MOV : instruction::mov;

				// prefix
				if (inst->fl & instruction::lock) {
					bytecode.append_byte(0xF0);
				}

				if (inst->fl & instruction::rep) {
					bytecode.append_byte(0xF3);
				}

				i32 dt = inst->dt;
				if (dt == x64::xmmword) {
					dt = x64::sse_pd;
				}

				// resolve output
				handle<value> out = context.create_value();
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
					const handle<value> left = context.create_value();
					i += resolve_interval(context, inst, i, left);
					ternary = (i < in_base + inst->in_count) || (inst->fl & (instruction::immediate | instruction::absolute));

					if (ternary && inst->ty == instruction::integral_multiplication && (inst->fl & instruction::immediate)) {
						ASSERT(false, "special case is not implemented");
					}

					if (inst->out_count == 0) {
						out = left;
					}
					else if (inst->ty == instruction::integral_div || inst->ty == instruction::div) {
						emit_instruction_1(context, inst->ty, left, dt, bytecode);
						continue;
					}
					else {
						if (out == nullptr) {
							out = context.create_value();
						}

						if (ternary || inst->ty == instruction::mov || inst->ty == instruction::FP_MOV) {
							if (out->matches(left) == false) {
								emit_instruction_2(context, static_cast<instruction::type>(mov_op), out, left, dt, bytecode);
							}
						}
						else {
							emit_instruction_2(context, inst->ty, out, left, dt, bytecode);
						}
					}
				}

				// unary ops
				if (cat == instruction::category::unary || cat == instruction::category::unary_ext) {
					emit_instruction_1(context, inst->ty, out, dt, bytecode);
					continue;
				}

				if (inst->fl & instruction::immediate) {
					const handle<value> right = value::create_imm(context, inst->get<immediate>().value);
					emit_instruction_2(context, inst->ty, out, right, dt, bytecode);
				}
				else if (inst->fl & instruction::absolute) {
					const handle<value> right = value::create_abs(context, inst->get<immediate>().value);
					emit_instruction_2(context, inst->ty, out, right, dt, bytecode);
				}
				else if (ternary) {
					const handle<value> right = context.create_value();
					resolve_interval(context, inst, i, right);

					if (inst->ty != instruction::mov || (inst->ty == instruction::mov && out->matches(right) == false)) {
						emit_instruction_2(context, inst->ty, out, right, dt, bytecode);
					}
				}
			}
		}
	}

	void x64_architecture::emit_function_epilogue(
		const codegen_context& context, utility::byte_buffer& bytecode
	) {
		ASSERT(context.function->exit_node != nullptr, "no exit node found");

		if(context.stack_usage <= 16) {
			bytecode.append_byte(0xC3);
			return;
		}

		// add RSP, stack_usage
		bytecode.append_byte(rex(true, 0x00, x64::rsp, 0));

		if (utility::fits_into_8_bits(context.stack_usage)) {
			bytecode.append_byte(0x83);
			bytecode.append_byte(mod_rx_rm(x64::direct, 0x00, x64::rsp));
			bytecode.append_byte(static_cast<i8>(context.stack_usage));
		}
		else {
			bytecode.append_byte(0x81);
			bytecode.append_byte(mod_rx_rm(x64::direct, 0x00, x64::rsp));
			bytecode.append_dword(static_cast<u32>(context.stack_usage));
		}

		// pop RBP
		bytecode.append_byte(0x58 + x64::rbp);

		// ret
		const handle<node> remote_procedure_call = context.function->exit_node->inputs[2];

		if(
			remote_procedure_call->ty == node::projection &&
			remote_procedure_call->inputs[0]->ty == node::entry &&
			remote_procedure_call->get<projection>().index == 2
		) {
			bytecode.append_byte(0xC3);
		}
	}

	auto x64_architecture::get_instruction_table() ->std::array<instruction::description, 120> {
		std::array<instruction::description, 120> table = {};

		table[instruction::RET        ] = { .mnemonic = "ret",       .cat = instruction::category::byte,     .op = 0xC3 };
		table[instruction::INT3       ] = { .mnemonic = "int3",      .cat = instruction::category::byte,     .op = 0xCC };
		table[instruction::STOSB      ] = { .mnemonic = "rep stosb", .cat = instruction::category::byte,     .op = 0xAA };
		table[instruction::MOVSB      ] = { .mnemonic = "rep movsb", .cat = instruction::category::byte,     .op = 0xA4 };
		table[instruction::CAST       ] = { .mnemonic = "cvt",       .cat = instruction::category::byte,     .op = 0x99 };
		table[instruction::system_call] = { .mnemonic = "syscall",   .cat = instruction::category::byte_ext, .op = 0x05 };
		table[instruction::RDTSC      ] = { .mnemonic = "rdtsc",     .cat = instruction::category::byte_ext, .op = 0x31 };
		table[instruction::UD2        ] = { .mnemonic = "ud2",       .cat = instruction::category::byte_ext, .op = 0x0B };

		table[instruction::NOT         ] = { .mnemonic = "not",  .cat = instruction::category::unary, .op = 0xF7, .rx_i = 0x02 };
		table[instruction::NEG         ] = { .mnemonic = "neg",  .cat = instruction::category::unary, .op = 0xF7, .rx_i = 0x03 };
		table[instruction::MUL         ] = { .mnemonic = "mul",  .cat = instruction::category::unary, .op = 0xF7, .rx_i = 0x04 };
		table[instruction::div         ] = { .mnemonic = "div",  .cat = instruction::category::unary, .op = 0xF7, .rx_i = 0x06 };
		table[instruction::integral_div] = { .mnemonic = "idiv", .cat = instruction::category::unary, .op = 0xF7, .rx_i = 0x07 };
		table[instruction::call        ] = { .mnemonic = "call", .cat = instruction::category::unary, .op = 0xE8, .rx_i = 0x02 };
		table[instruction::jmp         ] = { .mnemonic = "jmp",  .cat = instruction::category::unary, .op = 0xE9, .rx_i = 0x04 };

		// prefetching
		table[instruction::PREFETCHNTA] = { .mnemonic = "prefetchnta", .cat = instruction::category::unary_ext, .op = 0x18, .rx_i = 0 };
		table[instruction::PREFETCH0  ] = { .mnemonic = "prefetch0",   .cat = instruction::category::unary_ext, .op = 0x18, .rx_i = 1 };
		table[instruction::PREFETCH1  ] = { .mnemonic = "prefetch1",   .cat = instruction::category::unary_ext, .op = 0x18, .rx_i = 2 };
		table[instruction::PREFETCH2  ] = { .mnemonic = "prefetch2",   .cat = instruction::category::unary_ext, .op = 0x18, .rx_i = 3 };

		// jcc
		table[instruction::JO ] = { .mnemonic = "jo",  .cat = instruction::category::unary_ext, .op = 0x80 };
		table[instruction::JNO] = { .mnemonic = "jno", .cat = instruction::category::unary_ext, .op = 0x81 };
		table[instruction::JB] =  { .mnemonic = "jb",  .cat = instruction::category::unary_ext, .op = 0x82 };
		table[instruction::JNB] = { .mnemonic = "jnb", .cat = instruction::category::unary_ext, .op = 0x83 };
		table[instruction::JE] =  { .mnemonic = "je",  .cat = instruction::category::unary_ext, .op = 0x84 };
		table[instruction::JNE] = { .mnemonic = "jne", .cat = instruction::category::unary_ext, .op = 0x85 };
		table[instruction::JBE] = { .mnemonic = "jbe", .cat = instruction::category::unary_ext, .op = 0x86 };
		table[instruction::JA] =  { .mnemonic = "ja",  .cat = instruction::category::unary_ext, .op = 0x87 };
		table[instruction::JS] =  { .mnemonic = "js",  .cat = instruction::category::unary_ext, .op = 0x88 };
		table[instruction::JNS] = { .mnemonic = "jns", .cat = instruction::category::unary_ext, .op = 0x89 };
		table[instruction::JP] =  { .mnemonic = "jp",  .cat = instruction::category::unary_ext, .op = 0x8A };
		table[instruction::JNP] = { .mnemonic = "jnp", .cat = instruction::category::unary_ext, .op = 0x8B };
		table[instruction::JL] =  { .mnemonic = "jl",  .cat = instruction::category::unary_ext, .op = 0x8C };
		table[instruction::JGE] = { .mnemonic = "jge", .cat = instruction::category::unary_ext, .op = 0x8D };
		table[instruction::JLE] = { .mnemonic = "jle", .cat = instruction::category::unary_ext, .op = 0x8E };
		table[instruction::JG] =  { .mnemonic = "jg",  .cat = instruction::category::unary_ext, .op = 0x8F };

		// setcc
		table[instruction::SETO ] = { .mnemonic = "seto",  .cat = instruction::category::unary_ext, .op = 0x90 };
		table[instruction::SETNO] = { .mnemonic = "setno", .cat = instruction::category::unary_ext, .op = 0x91 };
		table[instruction::SETB ] = { .mnemonic = "setb",  .cat = instruction::category::unary_ext, .op = 0x92 };
		table[instruction::SETNB] = { .mnemonic = "setnb", .cat = instruction::category::unary_ext, .op = 0x93 };
		table[instruction::SETE ] = { .mnemonic = "sete",  .cat = instruction::category::unary_ext, .op = 0x94 };
		table[instruction::SETNE] = { .mnemonic = "setne", .cat = instruction::category::unary_ext, .op = 0x95 };
		table[instruction::SETBE] = { .mnemonic = "setbe", .cat = instruction::category::unary_ext, .op = 0x96 };
		table[instruction::SETA ] = { .mnemonic = "seta",  .cat = instruction::category::unary_ext, .op = 0x97 };
		table[instruction::SETS ] = { .mnemonic = "sets",  .cat = instruction::category::unary_ext, .op = 0x98 };
		table[instruction::SETNS] = { .mnemonic = "setns", .cat = instruction::category::unary_ext, .op = 0x99 };
		table[instruction::SETP ] = { .mnemonic = "setp",  .cat = instruction::category::unary_ext, .op = 0x9A };
		table[instruction::SETNP] = { .mnemonic = "setnp", .cat = instruction::category::unary_ext, .op = 0x9B };
		table[instruction::SETL ] = { .mnemonic = "setl",  .cat = instruction::category::unary_ext, .op = 0x9C };
		table[instruction::SETGE] = { .mnemonic = "setge", .cat = instruction::category::unary_ext, .op = 0x9D };
		table[instruction::SETLE] = { .mnemonic = "setle", .cat = instruction::category::unary_ext, .op = 0x9E };
		table[instruction::SETG ] = { .mnemonic = "setg",  .cat = instruction::category::unary_ext, .op = 0x9F };

		// cmovcc
		table[instruction::CMOVO ] = { .mnemonic = "cmovo",  .cat = instruction::category::binop_ext, .op = 0x40 };
		table[instruction::CMOVNO] = { .mnemonic = "cmovno", .cat = instruction::category::binop_ext, .op = 0x41 };
		table[instruction::CMOVB ] = { .mnemonic = "cmovb",  .cat = instruction::category::binop_ext, .op = 0x42 };
		table[instruction::CMOVNB] = { .mnemonic = "cmovnb", .cat = instruction::category::binop_ext, .op = 0x43 };
		table[instruction::CMOVE ] = { .mnemonic = "cmove",  .cat = instruction::category::binop_ext, .op = 0x44 };
		table[instruction::CMOVNE] = { .mnemonic = "cmovne", .cat = instruction::category::binop_ext, .op = 0x45 };
		table[instruction::CMOVBE] = { .mnemonic = "cmovbe", .cat = instruction::category::binop_ext, .op = 0x46 };
		table[instruction::CMOVA ] = { .mnemonic = "cmova",  .cat = instruction::category::binop_ext, .op = 0x47 };
		table[instruction::CMOVS ] = { .mnemonic = "cmovs",  .cat = instruction::category::binop_ext, .op = 0x48 };
		table[instruction::CMOVNS] = { .mnemonic = "cmovns", .cat = instruction::category::binop_ext, .op = 0x49 };
		table[instruction::CMOVP ] = { .mnemonic = "cmovp",  .cat = instruction::category::binop_ext, .op = 0x4A };
		table[instruction::CMOVNP] = { .mnemonic = "cmovnp", .cat = instruction::category::binop_ext, .op = 0x4B };
		table[instruction::CMOVL ] = { .mnemonic = "cmovl",  .cat = instruction::category::binop_ext, .op = 0x4C };
		table[instruction::CMOVGE] = { .mnemonic = "cmovge", .cat = instruction::category::binop_ext, .op = 0x4D };
		table[instruction::CMOVLE] = { .mnemonic = "cmovle", .cat = instruction::category::binop_ext, .op = 0x4E };
		table[instruction::CMOVG ] = { .mnemonic = "cmovg",  .cat = instruction::category::binop_ext, .op = 0x4F };

		// bit magic
		table[instruction::BSF] = { .mnemonic = "bsf", .cat = instruction::category::binop_ext, .op = 0xBC };
		table[instruction::BSF] = { .mnemonic = "bsr", .cat = instruction::category::binop_ext, .op = 0xBD };

		// binary ops but they have an implicit CL on the right-hand side
		table[instruction::SHL] = { .mnemonic = "shl", .cat = instruction::category::binop_cl, .op = 0xD2, .op_i = 0xC0, .rx_i = 0x04 };
		table[instruction::SHR] = { .mnemonic = "shr", .cat = instruction::category::binop_cl, .op = 0xD2, .op_i = 0xC0, .rx_i = 0x05 };
		table[instruction::ROL] = { .mnemonic = "rol", .cat = instruction::category::binop_cl, .op = 0xD2, .op_i = 0xC0, .rx_i = 0x00 };
		table[instruction::ROR] = { .mnemonic = "ror", .cat = instruction::category::binop_cl, .op = 0xD2, .op_i = 0xC0, .rx_i = 0x01 };
		table[instruction::SAR] = { .mnemonic = "sar", .cat = instruction::category::binop_cl, .op = 0xD2, .op_i = 0xC0, .rx_i = 0x07 };

		table[instruction::ADD ] =  { .mnemonic = "add",  .cat = instruction::category::binop, .op = 0x00, .op_i = 0x80, .rx_i = 0x00 };
		table[instruction::OR  ] =  { .mnemonic = "or",   .cat = instruction::category::binop, .op = 0x08, .op_i = 0x80, .rx_i = 0x01 };
		table[instruction::AND ] =  { .mnemonic = "and",  .cat = instruction::category::binop, .op = 0x20, .op_i = 0x80, .rx_i = 0x04 };
		table[instruction::SUB ] =  { .mnemonic = "sub",  .cat = instruction::category::binop, .op = 0x28, .op_i = 0x80, .rx_i = 0x05 };
		table[instruction::XOR ] =  { .mnemonic = "xor",  .cat = instruction::category::binop, .op = 0x30, .op_i = 0x80, .rx_i = 0x06 };
		table[instruction::CMP ] =  { .mnemonic = "cmp",  .cat = instruction::category::binop, .op = 0x38, .op_i = 0x80, .rx_i = 0x07 };
		table[instruction::mov ] =  { .mnemonic = "mov",  .cat = instruction::category::binop, .op = 0x88, .op_i = 0xC6, .rx_i = 0x00 };
		table[instruction::TEST] =  { .mnemonic = "test", .cat = instruction::category::binop, .op = 0x84, .op_i = 0xF6, .rx_i = 0x00 };

		// misc integer ops
		table[instruction::movabs                 ] =  { .mnemonic = "mov",    .cat = instruction::category::binop_plus, .op = 0xB8 };
		table[instruction::XCHG                   ] =  { .mnemonic = "xchg",   .cat = instruction::category::binop,      .op = 0x86 };
		table[instruction::lea                    ] =  { .mnemonic = "lea",    .cat = instruction::category::binop,      .op = 0x8D };
		table[instruction::XADD                   ] =  { .mnemonic = "xadd",   .cat = instruction::category::binop_ext,  .op = 0xC0 };
		table[instruction::integral_multiplication] =  { .mnemonic = "imul",   .cat = instruction::category::binop_ext,  .op = 0xAF };
		table[instruction::IMUL3                  ] =  { .mnemonic = "imul",   .cat = instruction::category::binop,      .op = 0x69 };
		table[instruction::MOVSXB                 ] =  { .mnemonic = "movsxb", .cat = instruction::category::binop_ext2, .op = 0xBE };
		table[instruction::MOVSXW                 ] =  { .mnemonic = "movsxw", .cat = instruction::category::binop_ext2, .op = 0xBF };
		table[instruction::MOVSXD                 ] =  { .mnemonic = "movsxd", .cat = instruction::category::binop,      .op = 0x63 };
		table[instruction::MOVZXB                 ] =  { .mnemonic = "movzxb", .cat = instruction::category::binop_ext2, .op = 0xB6 };
		table[instruction::MOVZXW                 ] =  { .mnemonic = "movzxw", .cat = instruction::category::binop_ext2, .op = 0xB7 };

		// gpr<->xmm
		table[instruction::MOV_I2F] = { .mnemonic = "mov", .cat = instruction::category::binop_ext3, .op = 0x6E };
		table[instruction::MOV_F2I] = { .mnemonic = "mov", .cat = instruction::category::binop_ext3, .op = 0x7E };

		// SSE binary operations
		table[instruction::FP_MOV            ] = {  .mnemonic ="mov",   .cat = instruction::category::binop_sse, .op = 0x10 };
		table[instruction::FP_ADD            ] = {  .mnemonic ="add",   .cat = instruction::category::binop_sse, .op = 0x58 };
		table[instruction::FP_MUL            ] = {  .mnemonic ="mul",   .cat = instruction::category::binop_sse, .op = 0x59 };
		table[instruction::FP_SUB            ] = {  .mnemonic ="sub",   .cat = instruction::category::binop_sse, .op = 0x5C };
		table[instruction::FP_MIN            ] = {  .mnemonic ="min",   .cat = instruction::category::binop_sse, .op = 0x5D };
		table[instruction::FP_DIV            ] = {  .mnemonic ="div",   .cat = instruction::category::binop_sse, .op = 0x5E };
		table[instruction::FP_MAX            ] = {  .mnemonic ="max",   .cat = instruction::category::binop_sse, .op = 0x5F };
		table[instruction::FP_CMP            ] = {  .mnemonic ="cmp",   .cat = instruction::category::binop_sse, .op = 0xC2 };
		table[instruction::FP_UCOMI          ] = {  .mnemonic ="ucomi", .cat = instruction::category::binop_sse, .op = 0x2E };
		table[instruction::FP_CVT32          ] = {  .mnemonic ="cvtsi", .cat = instruction::category::binop_sse, .op = 0x2A };
		table[instruction::FP_CVT64          ] = {  .mnemonic ="cvtsi", .cat = instruction::category::binop_sse, .op = 0x2A };
		table[instruction::FP_CVT            ] = {  .mnemonic ="cvt",   .cat = instruction::category::binop_sse, .op = 0x5A };
		table[instruction::FP_CVTT           ] = {  .mnemonic ="rsqrt", .cat = instruction::category::binop_sse, .op = 0x2C };
		table[instruction::FP_SQRT           ] = {  .mnemonic ="and",   .cat = instruction::category::binop_sse, .op = 0x51 };
		table[instruction::FP_RSQRT          ] = {  .mnemonic ="or",    .cat = instruction::category::binop_sse, .op = 0x52 };
		table[instruction::FP_AND            ] = {  .mnemonic ="xor",   .cat = instruction::category::binop_sse, .op = 0x54 };
		table[instruction::FP_OR             ] = {  .mnemonic ="or",    .cat = instruction::category::binop_sse, .op = 0x56 };
		table[instruction::FP_XOR            ] = {  .mnemonic ="xor",   .cat = instruction::category::binop_sse, .op = 0x57 };

		return table;
	}

	void x64_architecture::emit_instruction_0(
		instruction::type type, i32 data_type, utility::byte_buffer& bytecode
	) {
		ASSERT(type < s_instruction_table.size(), "type is out of range");
		const auto& description = &s_instruction_table[type];

		if (data_type == x64::qword) {
			bytecode.append_byte(0x48);
		}

		if (description->cat == instruction::category::byte_ext) {
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
		instruction::type type, 
		handle<value> r,
		i32 dt, 
		utility::byte_buffer& bytecode
	) {
		ASSERT(type < s_instruction_table.size(), "invalid type");
		const instruction::description& descriptor = s_instruction_table[type];

		const bool is_rex = dt == x64::byte || dt == x64::qword;
		const bool is_rexw = dt == x64::qword;
		const u8 op = descriptor.op_i;
		const u8 rx = descriptor.rx_i;

		if (r->m_type == value::gpr) {
			if (is_rex || r->m_reg >= 8) {
				bytecode.append_byte(rex(is_rexw, 0x00, r->m_reg, 0x00));
			}

			if (descriptor.cat == instruction::category::unary_ext) {
				bytecode.append_byte(0x0F);
			}

			bytecode.append_byte(op ? op : descriptor.op);
			bytecode.append_byte(mod_rx_rm(x64::direct, rx, r->m_reg));
		}
		else if (r->m_type == value::mem) {
			const i32 displacement = r->imm;
			const u8 index = r->m_index;
			const u8 base = r->m_reg;
			scale s = r->m_scale;

			const bool needs_index = (index != reg::invalid_id) || (base & 7) == x64::rsp;
			bytecode.append_byte(rex(is_rexw, 0x00, base, index != reg::invalid_id ? index : 0));

			if (descriptor.cat == instruction::category::unary_ext) {
				bytecode.append_byte(0x0F);
			}

			bytecode.append_byte(op);

			// if it needs an index, it'll put RSP into the base slot
			// and write the real base into the SIB
			x64::mod m = x64::indirect_displacement_32;

			if (utility::fits_into_8_bits(displacement)) {
				m = x64::indirect_displacement_8;
			}

			bytecode.append_byte(mod_rx_rm(m, rx, needs_index ? x64::rsp : base));

			if (needs_index) {
				bytecode.append_byte(mod_rx_rm(static_cast<x64::mod>(s), (base & 7) == x64::rsp ? x64::rsp : index, base));
			}

			if (m == x64::indirect_displacement_8) {
				bytecode.append_byte(static_cast<i8>(displacement));
			}
			else if (m == x64::indirect_displacement_32) {
				bytecode.append_dword(static_cast<i32>(displacement));
			}
		}
		else if (r->m_type == value::global) {
			if(descriptor.op) {
				if(descriptor.cat == instruction::category::unary_ext) {
					bytecode.append_byte(0x0F);
				}

				bytecode.append_byte(descriptor.op);
			}
			else {
				if(is_rex) {
					bytecode.append_byte(is_rexw ? 0x48 : 0x40);
				}

				if (descriptor.cat == instruction::category::unary_ext) {
					bytecode.append_byte(0x0F);
				}

				bytecode.append_byte(op);
				bytecode.append_byte(((rx & 7) << 3) | x64::rbp);
			}

			bytecode.append_dword(r->m_imm);
			emit_symbol_patch(context, r->get<handle<symbol>>(), bytecode.get_size() - 4);
		}
		else if (r->m_type == value::label) {
			if (descriptor.cat == instruction::category::unary_ext) {
				bytecode.append_byte(0x0F);
			}

			bytecode.append_byte(descriptor.op);
			bytecode.append_dword(0);

			const u64 label = r->get<ir::label>().value;
			bytecode.emit_relocation_dword(&context.labels[label], bytecode.get_size() - 4);
		}
		else {
			ASSERT(false, "not implemented");
		}
	}

	void x64_architecture::emit_instruction_2(
		codegen_context& context,
		instruction::type type,
		handle<value> a,
		handle<value> b, 
		i32 data_type,
		utility::byte_buffer& bytecode
	) {
		if (data_type >= x64::sse_ss && data_type <= x64::sse_pd) {
			ASSERT(false, "");
		}

		ASSERT(data_type >= x64::byte && data_type <= x64::qword, "invalid data type");
		ASSERT(type < s_instruction_table.size(), "invalid type");

		const instruction::description& descriptor = s_instruction_table[type];
		const bool dir = b->m_type == value::mem || b->m_type == value::global;

		if (type == instruction::movabs) {
			ASSERT(
				a->m_type == value::gpr && b->m_type == value::abs,
				"invalid data types for a movabs operation"
			);

			bytecode.append_byte(rex(true, a->m_reg, 0, 0));
			bytecode.append_byte(descriptor.op + (a->m_reg & 0b111));
			bytecode.append_qword(b->get<absolute>().value);
		}

		if (
			dir ||
			descriptor.op == 0x63 ||
			descriptor.op == 0x69 ||
			descriptor.op == 0x6E ||
			(type >= instruction::CMOVO && type <= instruction::CMOVG) ||
			descriptor.op == 0xAF ||
			descriptor.cat == instruction::category::binop_ext2
		) {
			std::swap(a, b);
		}

		// operand size 
		bool sz = data_type != x64::byte;

		// uses an imm value that works as a sign extended 8 bit number
		const bool short_imm =
			sz && b->m_type == value::imm &&
			b->m_imm == static_cast<i8>(b->m_imm) &&
			descriptor.op_i == 0x80;

		// the destination can only be a GPR, no direction flag
		const bool is_gpr_only_dst = descriptor.op & 1;
		const bool dir_flag = dir != is_gpr_only_dst && descriptor.op != 0x69;

		if (descriptor.cat != instruction::category::binop_ext3) {
			// address size prefix
			if (data_type == x64::word && descriptor.cat != instruction::category::binop_ext2) {
				bytecode.append_byte(0x66);
			}

			ASSERT(
				b->m_type == value::gpr || b->m_type == value::imm,
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

		u8 rex_prefix = 0x40 | (data_type == x64::qword ? 8 : 0);
		u8 base;

		if (a->m_type == value::mem || a->m_type == value::gpr) {
			base = a->m_reg;
		}
		else {
			base = x64::rbp;
		}

		if (a->m_type == value::mem && a->m_index != reg::invalid_id) {
			rex_prefix |= ((a->m_index >> 3) << 1);
		}

		u8 rx = (b->m_type == value::gpr || b->m_type == value::xmm) ? b->m_reg : descriptor.rx_i;

		if (descriptor.cat == instruction::category::binop_cl) {
			ASSERT(
				b->m_type == value::imm || (b->m_type == value::gpr && b->m_reg == x64::rcx),
				"invalid binary operation"
			);

			data_type = x64::byte;
			rx = descriptor.rx_i;
		}

		rex_prefix |= (base >> 3);
		rex_prefix |= (rx >> 3) << 2;

		// if the REX stays as 0x40 then it's default and doesn't need to be here
		if (rex_prefix != 0x40 || data_type == x64::byte || type == instruction::MOVZXB) {
			bytecode.append_byte(rex_prefix);
		}

		if (descriptor.cat == instruction::category::binop_ext3) {
			bytecode.append_byte(0x0F);
			bytecode.append_byte(descriptor.op);
		}
		else {
			// opcode
			if (
				descriptor.cat == instruction::category::binop_ext || 
				descriptor.cat == instruction::category::binop_ext2
			) {
				// DEF instructions can only be 32bit and 64bit... maybe?
				if (type != instruction::XADD) {
					sz = false;
				}

				bytecode.append_byte(0x0F);
			}

			// immediates have a custom opcode
			ASSERT(
				b->m_type != value::imm || descriptor.op_i != 0 || descriptor.rx_i != 0, 
				"no immediate variant of instruction"
			);

			u8 opcode = b->m_type == value::imm ? descriptor.op_i : descriptor.op;

			// the bottom bit usually means size, 0 for 8bit, 1 for everything else
			opcode |= sz;

			// you can't actually be flipped in the immediates because it would mean
			// you're storing into an immediate so they reuse that direction bit for size
			opcode |= dir_flag << 1;
			opcode |= short_imm << 1;

			bytecode.append_byte(opcode);
		}

		emit_memory_operand(context, rx, a, bytecode);

		// memory displacements go before immediates
		const u64 disp_patch = bytecode.get_size() - 4;

		if (b->m_type == value::imm) {
			if (data_type == x64::byte || short_imm) {
				if (short_imm) {
					ASSERT(
						b->m_imm == static_cast<i8>(b->m_imm), 
						"invalid short immediate"
					);
				}

				bytecode.append_byte(static_cast<i8>(b->m_imm));
			}
			else if (data_type == x64::word) {
				const i32 imm = b->m_imm;
				ASSERT(
					(imm & 0xFFFF0000) == 0xFFFF0000 || (imm & 0xFFFF0000) == 0,
					"invalid immediate"
				);

				bytecode.append_word(static_cast<u16>(imm));
			}
			else {
				bytecode.append_dword(b->m_imm);
			}
		}

		if (a->m_type == value::global && 
			disp_patch + 4 != bytecode.get_size()
		) {
			bytecode.patch_dword(
				disp_patch, static_cast<u32>(disp_patch + 4 - bytecode.get_size())
			);
		}
	}

	void x64_architecture::emit_memory_operand(
		codegen_context& context, u8 rx, handle<value> a, utility::byte_buffer& bytecode
	) {
		// operand encoding
		if (a->m_type == value::gpr || a->m_type == value::xmm) {
			bytecode.append_byte(mod_rx_rm(x64::direct, rx, a->m_reg));
		}
		else if (a->m_type == value::mem) {
			const u8 base = a->m_reg;
			const u8 index = a->m_index;
			scale scale = a->m_scale;
			const i32 displacement = a->m_imm;
			const bool needs_index = (index != reg::invalid_id) || (base & 7) == x64::rsp;
			
			// if it needs an index, it'll put RSP into the base slot and write the real base into the SIB
			x64::mod m = x64::indirect_displacement_32;
			
			if (displacement == 0 && (base & 7) != x64::rbp) {
				m = x64::indirect;
			}
			else if (displacement == static_cast<i8>(displacement)) {
				m = x64::indirect_displacement_8;
			}

			bytecode.append_byte(mod_rx_rm(m, rx, needs_index ? x64::rsp : base));
			
			if (needs_index) {
				bytecode.append_byte(mod_rx_rm(static_cast<x64::mod>(scale), (base & 7) == x64::rsp ? x64::rsp : index, base));
			}
			
			if (m == x64::indirect_displacement_8) {
				bytecode.append_byte(static_cast<i8>(displacement));
			}
			else if (m == x64::indirect_displacement_32) {
				bytecode.append_dword(displacement);
			}
		}
		else if (a->m_type == value::global) {
			bytecode.append_byte(((rx & 7) << 3) | x64::rbp);
			bytecode.append_dword(a->m_imm);
			emit_symbol_patch(context, a->get<handle<symbol>>(), bytecode.get_size() - 4);
		}
		else {
			ASSERT(false, "not implemented 2");
		}
	}

	auto x64_architecture::resolve_interval(
		const codegen_context& context,
		handle<instruction> inst,
		u8 i,
		handle<value> val
	) -> u8 {
		handle interval = &context.intervals[inst->operands[i]];

		if (
			(inst->fl & (instruction::mem_f |instruction::global)) &&
			i == inst->memory_slot
		) {
			ASSERT(interval->spill <= 0, "cannot use spilled value for a memory operand");

			if (inst->fl & instruction::mem_f) {
				val->m_type = value::mem;
				val->m_reg = interval->assigned.id;
				val->m_index = reg::invalid_id;
				val->m_scale = inst->sc;
				val->m_imm = inst->displacement;

				if (inst->fl & instruction::indexed) {
					interval = &context.intervals[inst->operands[i + 1]];
					ASSERT(interval->spill <= 0, "cannot use spilled value for a memory operand");

					val->m_index = interval->assigned.id;
					return 2;
				}

				return 1;
			}

			val->m_type = value::global;
			val->m_imm = inst->displacement;
			val->get<handle<symbol>>() = inst->get<handle<symbol>>();

			return 1;
		}

		if (interval->spill > 0) {
			val->m_type = value::mem;
			val->m_reg = x64::rbp;
			val->m_index = reg::invalid_id;
			val->m_imm = -interval->spill;
		}
		else {
			val->m_type = interval->reg.cl == x64::register_class::xmm ? value::xmm : value::gpr;
			val->m_reg = interval->assigned.id;
		}

		return 1;
	}

	void x64_architecture::emit_symbol_patch(
		codegen_context& context, handle<symbol> target, u64 pos
	) {
		// TODO: fix this memory leak
		handle p = new symbol_patch();
		p->next = nullptr;
		p->target = target;
		p->pos = pos;

		context.patch_count++;

		if(context.first_patch == nullptr) {
			context.first_patch = context.last_patch = p;
		}
		else {
			context.last_patch->next = p;
			context.last_patch = p;
		}
	}

	auto rex(bool is_64_bit, u8 rx, u8 base, u8 index) -> u8 {
		return 0x40 | (is_64_bit ? 8 : 0) | (base >> 3) | ((index >> 3) << 1) | ((rx >> 3) << 2);
	}

	auto mod_rx_rm(x64::mod mod, u8 rx, u8 rm) -> u8 {
		return ((mod & 3) << 6) | ((rx & 7) << 3) | (rm & 7);
	}
}
