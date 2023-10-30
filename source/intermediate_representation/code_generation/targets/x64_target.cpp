#include "x64_target.h"
#include "intermediate_representation/code_generation/code_generator_context.h"

namespace ir::cg {
	utility::block_allocator value::s_value_allocator = utility::block_allocator(4096);

	void x64_target::emit_code(
		code_generator_context& context, 
		s_ptr<cg::code_generation_result> result
	) {
		// TODO: replace by a unified output struct 
		m_assembly_output = &result->assembly_output;

		// resolve stack usage
		context.stack_usage = utility::align(context.stack_usage, 16);

		emit_prologue(context, result->bytecode);
		emit_function_body(context, result->bytecode);
		emit_epilogue(context, result->bytecode);
	}

	void x64_target::allocate_base_registers(
		code_generator_context& context
	) {
		context.intervals.reserve(2 * 16);

		// initialize general purpose registers
		for (u64 i = 0; i < 16; ++i) {
			context.intervals.emplace_back(
				live_interval(
					reg(static_cast<i32>(i), static_cast<i32>(register_class::gpr)),
					qword, static_cast<i32>(i)
				)
			);
		}

		// initialize xmm registers
		for (u64 i = 0; i < 16; ++i) {
			context.intervals.emplace_back(
				live_interval(
					reg(static_cast<i32>(i), static_cast<i32>(register_class::xmm)),
					xmmword, static_cast<i32>(i)
				)
			);
		}
	}

	i32 x64_target::legalize_data_type(const data_type& data_type) {
		if(data_type.get_id() == data_type::id::floating_point) {
			ASSERT(false, "not implemented");
		}

		u64 mask;
		return legalize_integer_data_type(&mask, data_type);
	}

	i32 x64_target::classify_register_class(const data_type& data_type) {
		return data_type.get_id() == data_type::id::floating_point ? static_cast<i32>(register_class::xmm) : static_cast<i32>(register_class::gpr);
	}

	void x64_target::emit_prologue(
		code_generator_context& context,
		utility::byte_buffer& bytecode
	) {
		emit_assembly("{}:\n", context.function->get_symbol().get_name());

		if(context.stack_usage <= 16) {
			return;
		}

		// push RBP
		emit_assembly("  push RBP\n");
		bytecode.append_byte(0x50 + static_cast<u8>(gpr::rbp));

		// mov RBP, RSP
		emit_assembly("  mov RBP, RSP\n");
		bytecode.append_byte(rex(true, static_cast<u8>(gpr::rsp), static_cast<u8>(gpr::rbp), 0));
		bytecode.append_byte(0x89);
		bytecode.append_byte(mod_rx_rm(direct, static_cast<u8>(gpr::rsp), static_cast<u8>(gpr::rbp)));

		// if there's more than 4096 bytes of stack, we need to insert a chkstk
		// function
		if(context.stack_usage >= 4096) {
			ASSERT(false, "not implemented");
		}
		else {
			emit_assembly("  sub RSP, {}\n", context.stack_usage);
			bytecode.append_byte(rex(true, 0x00, static_cast<u8>(gpr::rsp), 0));

			if (context.stack_usage == static_cast<i8>(context.stack_usage)) {
				// sub RSP, stack_usage
				bytecode.append_byte(0x83);
				bytecode.append_byte(mod_rx_rm(direct, 0x05, static_cast<u8>(gpr::rsp)));
				bytecode.append_byte(static_cast<u8>(context.stack_usage));
			}
			else {
				// sub RSP, stack_usage
				bytecode.append_byte(0x81);
				bytecode.append_byte(mod_rx_rm(direct, 0x05, static_cast<u8>(gpr::rsp)));
				bytecode.append_dword(static_cast<u32>(context.stack_usage));
			}
		}
	}

	void x64_target::emit_function_body(
		code_generator_context& context,
		utility::byte_buffer& bytecode
	) {
		for (handle<instruction> inst = context.first; inst; inst = inst->get_next_instruction()) {
			const u64 in_base = inst->get_out_count();
			const instruction::category cat = inst->get_type() >= g_x64_instruction_table.size() ?
				instruction::category::binop :
				g_x64_instruction_table[inst->get_type()].cat;

			if (
				inst->get_type() == instruction::entry ||
				inst->get_type() == instruction::terminator
			) {
				// does nothing
			}
			else if (inst->get_type() == instruction::type::label) {
				handle<node> block = inst->get<node_prop>()->value;

				bytecode.resolve_relocation_dword(
					&context.labels.at(block), 
					static_cast<u32>(bytecode.get_size())
				);

				if(block != context.function->get_entry_node()) {
					ASSERT(block->get_type() == node::region, "invalid block type");
					emit_assembly("L{}:\n", block->get<region_property>()->post_order_id);
				}
			}
			else if (inst->get_type() == instruction::type::inl) {
				ASSERT(false, "not implemented 3");
			}
			else if (inst->get_type() == instruction::type::epilogue) {
				// return label goes here
				emit_assembly(".ret:\n");
				bytecode.resolve_relocation_dword(
					&context.return_label,
					static_cast<u32>(bytecode.get_size())
				);
			}
			else if (inst->get_type() == instruction::type::line) {
				ASSERT(false, "not implemented 5");
			}
			else if (
				cat == instruction::category::byte ||
				cat == instruction::category::byte_ext
			) {
				if(inst->get_flags() & instruction::rep) {
					bytecode.append_byte(0xF3);
				}

				emit_assembly("  {}\n", g_x64_instruction_table[inst->get_type()].mnemonic);
				emit_instruction_0(inst->get_type(), inst->get_data_type(), bytecode);
			}
			else if (inst->get_type() == instruction::type::zero) {
				ASSERT(false, "not implemented 7");
			}
			else if (
				inst->get_type() >= instruction::type::jmp &&
				inst->get_type() <= instruction::type::JG
			) {
				handle<value> target;
				if(inst->get_flags() & instruction::node) {
					target = value::create_label(inst->get<node_prop>()->value);
				}
				else {
					ASSERT(false, "not implemented");
				}

				emit_instruction_1_print(inst->get_type(), target, inst->get_data_type(), bytecode);
			}
			else if (inst->get_type() == instruction::type::call) {
				ASSERT(false, "not implemented 9");
			}
			else {
				i32 mov_op = (inst->get_data_type() >= pbyte && inst->get_data_type() <= xmmword) ?
					instruction::floating_point_mov : instruction::mov;

				// prefix
				if(inst->get_flags() & instruction::lock) {
					emit_assembly("  LOCK");
					bytecode.append_byte(0xF0);
				}

				// resolve output
				handle<value> out = value::create_value<empty_property>();
				i32 i = 0;

				if(inst->get_out_count() == 1) {
					i += resolve_interval(context, inst, i, out);
					ASSERT(static_cast<u64>(i) == in_base, "invalid base");
				}
				else {
					i = static_cast<i32>(in_base);
				}

				// first parameter
				bool ternary = false;
				if(inst->get_in_count() > 0) {
					const handle<value> left = value::create_value<empty_property>();
					i += resolve_interval(context, inst, i, left);

					ternary =
						(i < (static_cast<i32>(in_base) + static_cast<i32>(inst->get_in_count()))) ||
						(inst->get_flags() & (instruction::immediate | instruction::absolute));

					if (
						ternary &&
						inst->get_type() == instruction::type::integral_multiplication &&
						inst->get_flags() & instruction::immediate
					) {
						// there's a special case for ternary IMUL r64, r/m64, imm32
						ASSERT(false, "not implemented");
						continue;
					}

					if (inst->get_out_count() == 0) {
						out = left;
					}
					else if (
						inst->get_type() == instruction::type::integral_div ||
						inst->get_type() == instruction::type::div
					) {
						emit_instruction_1_print(inst->get_type(), left, inst->get_data_type(), bytecode);
						continue;
					}
					else {
						if (
							ternary ||
							inst->get_type() == instruction::mov ||
							inst->get_type() == instruction::floating_point_mov
						) {
							if (out->matches(left) == false) {
								emit_instruction_2_print(static_cast<instruction::type>(mov_op), out, left, inst->get_data_type(), bytecode);
							}
						}
						else {
							emit_instruction_2_print(inst->get_type(), out, left, inst->get_data_type(), bytecode);
						}
					}
				}

				// unary ops
				if(cat == instruction::unary || cat == instruction::unary_ext) {
					emit_instruction_1_print(inst->get_type(), out, inst->get_data_type(), bytecode);
					continue;
				}

				if(inst->get_flags() & instruction::immediate) {
					const handle<value> right = value::create_imm(inst->get<immediate_prop>()->value);
					emit_instruction_2_print(inst->get_type(), out, right, inst->get_data_type(), bytecode);
				}
				else if (inst->get_flags() & instruction::absolute) {
					ASSERT(false, "not implemented abs");
				}
				else if (ternary) {
					const handle<value> right = value::create_value<empty_property>();
					resolve_interval(context, inst, i, right);

					if(
						inst->get_type() != instruction::mov ||
						(inst->get_type() == instruction::mov && out->matches(right) == false)
					) {
						emit_instruction_2_print(inst->get_type(), out, right, inst->get_data_type(), bytecode);
					}
				}
			}
		}
	}

	void x64_target::emit_epilogue(
		code_generator_context& context,
		utility::byte_buffer& bytecode
	) {
		if(context.stack_usage <= 16) {
			emit_assembly("  ret\n");
			bytecode.append_byte(0xC3);
			return;
		}

		// add RSP, stack_usage
		emit_assembly("  add RSP, {}\n", context.stack_usage);
		bytecode.append_byte(rex(true, 0x00, static_cast<u8>(gpr::rsp), 0));
		if (context.stack_usage == static_cast<i8>(context.stack_usage)) {
			bytecode.append_byte(0x83);
			bytecode.append_byte(mod_rx_rm(direct, 0x00, static_cast<u8>(gpr::rsp)));
			bytecode.append_byte(static_cast<i8>(context.stack_usage));
		}
		else {
			bytecode.append_byte(0x81);
			bytecode.append_byte(mod_rx_rm(direct, 0x00, static_cast<u8>(gpr::rsp)));
			bytecode.append_dword(static_cast<u32>(context.stack_usage));
		}

		// pop RBP
		emit_assembly("  pop RBP\n");
		bytecode.append_byte(0x58 + static_cast<u8>(gpr::rbp));

		// ret
		const handle<node> rpc = context.function->get_exit_node()->get_input(2);

		if (
			rpc->get_type() == node::projection &&
			rpc->get_input(0)->get_type() == node::entry &&
			rpc->get<projection_property>()->index == 2
		) {
			emit_assembly("  ret\n");
			bytecode.append_byte(0xC3);
		}
	}

	void x64_target::emit_instruction_0(
		instruction::type type, i32 data_type, utility::byte_buffer& bytecode
	) {
		ASSERT(type < g_x64_instruction_table.size(), "type out of range");
		const instruction::description* description = &g_x64_instruction_table[type];

		if(data_type == qword) {
			bytecode.append_byte(0x48);
		}

		if(description->cat == instruction::byte_ext) {
			bytecode.append_byte(0x0F);
		}

		if(description->op) {
			bytecode.append_byte(description->op);
		}
		else {
			bytecode.append_byte(description->op);
			bytecode.append_byte(description->rx_i);
		}
	}

	void x64_target::emit_instruction_1_print(
		instruction::type type, handle<value> src, i32 data_type, utility::byte_buffer& bytecode
	) {
		emit_assembly("  {} ", g_x64_instruction_table[type].mnemonic);
		print_operand(src, data_type);
		emit_assembly("\n");

		// TODO: emit_instruction_1
	}

	void x64_target::emit_instruction_2_print(
		instruction::type type, 
		handle<value> dst,
		handle<value> src,
		i32 data_type, 
		utility::byte_buffer& bytecode
	) {
		if(data_type == xmmword) {
			data_type = sse_pd;
		}

		emit_assembly("  {}", g_x64_instruction_table[static_cast<i32>(type)].mnemonic);
		if(g_x64_instruction_table[type].cat == instruction::binop_ext3) {
			emit_assembly("{} ", data_type == qword ? 'q' : 'd');
		}
		else if(data_type >= sse_ss && data_type <= sse_pd) {
			static constexpr char suffixes[4][3] = { "ss", "sd", "ps", "pd" };
			emit_assembly("{} ", suffixes[data_type - sse_ss]);
		}
		else {
			emit_assembly(" ");
		}

		print_operand(dst, data_type);
		emit_assembly(", ");
		print_operand(src, data_type);
		emit_assembly("\n");

		if(data_type >= sse_ss && data_type <= sse_pd) {
			ASSERT(false, "not implemented");
		}
		else {
			// TODO: emit_instruction_2
		}
	}

	void x64_target::print_operand(handle<value> val, i32 data_type) {
		static const char* type_names[] = {
			"ptr",
			"byte",    "word",    "dword",   "qword",
			"pbyte",   "pword",   "pdword",  "pqword",
			"xmmword", "xmmword", "xmmword", "xmmword", "xmmword"
		};

		static const char* gpr_names[] = {
			"RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI", "RDI",
			"R8",  "R9",  "R10", "R11", "R12", "R13", "R14", "R15"
		};
		
		switch (val->get_type()) {
			case value::gpr: {
				ASSERT(
					val->get_reg() >= 0 && val->get_reg() < 16, 
					"invalid register"
				);

				emit_assembly("{}", gpr_names[val->get_reg()]);
				break;
			}
			case value::xmm: {
				emit_assembly("XMM{}", val->get_reg());
				break;
			}
			case value::imm: {
				emit_assembly("{}", val->get_imm());
				break;
			}
			case value::abs: {
				emit_assembly("{}", val->get<abs_prop>()->value);
				break;
			}
			case value::mem: {
				emit_assembly("{} ", type_names[data_type]);

				if (val->get_index() == -1) {
					emit_assembly("[{}", gpr_names[val->get_reg()]);
				}
				else {
					emit_assembly("[{} + {} * {}", gpr_names[val->get_reg()], gpr_names[val->get_index()], 1u << static_cast<unsigned>(val->get_scale()));
				}

				if (val->get_imm() != 0) {
					emit_assembly(" + {}", val->get_imm());
				}

				emit_assembly("]");
				break;
			}
			case value::global: {
				ASSERT(false, "not implemented - global");
			}
			case value::label: {
				const handle<node> target = val->get<target_prop>()->target;

				if (target == nullptr) {
					emit_assembly(".ret");
				}
				else {
					ASSERT(
						target->get_type() == node::entry || target->get_type() == node::region,
						"invalid node type"
					);

					emit_assembly("L{}", target->get<region_property>()->post_order_id);
				}
				break;
			}
			default: {
				ASSERT(false, "not implemented xxx");
			}
		}
	}

	i32 x64_target::resolve_interval(
		code_generator_context& context, 
		handle<instruction> inst,
		i32 i, 
		handle<value> val
	) {
		handle interval = &context.intervals[inst->get_operand(i)];

		if(
			(inst->get_flags() & (instruction::mem | instruction::global)) && 
			i == inst->get_memory_slot()
		) {
			ASSERT(interval->get_spill() <= 0, "cannot use spilled value for a memory operand");

			if(inst->get_flags() & instruction::mem) {
				val->set_type(value::mem);
				val->set_reg(static_cast<i8>(interval->get_assigned()));
				val->set_index(static_cast<i8>(gpr::none));
				val->set_scale(static_cast<scale>(inst->get_scale()));
				val->set_imm(inst->get_disp());

				if(inst->get_flags() & instruction::indexed) {
					interval = &context.intervals[inst->get_operand(i + 1)];
					ASSERT(interval->get_spill() <= 0, "cannot use spilled value for a memory operand");

					val->set_index(static_cast<i8>(interval->get_assigned()));
					return 2;
				}

				return 1;
			}
			else {
				ASSERT(false, "not implemented - global");
				return 1;
			}
		}

		if(interval->get_spill() > 0) {
			val->set_type(value::mem);
			val->set_reg(static_cast<i8>(gpr::rbp));
			val->set_index(static_cast<i8>(gpr::none));
			val->set_imm(-interval->get_spill());
		}
		else {
			val->set_type(static_cast<register_class>(
				interval->get_register().get_class()) == register_class::xmm ? value::xmm : value::gpr
			);

			val->set_reg(static_cast<i8>(interval->get_assigned()));
		}

		return 1;
	}

	u8 x64_target::rex(bool is_64bit, u8 rx, u8 base, u8 index) {
		return 0x40 | (is_64bit ? 8 : 0) | (base >> 3) | ((index >> 3) << 1) | ((rx >> 3) << 2);
	}

	u8 x64_target::mod_rx_rm(mod mod, u8 rx, u8 rm) {
		return ((mod & 3) << 6) | ((rx & 7) << 3) | (rm & 7);
	}

	x64_data_type x64_target::legalize_integer_data_type(u64* out_mask, const data_type& data_type) {
		const data_type::id type = data_type.get_id();

		ASSERT(
			type == data_type::id::integer || type == data_type::id::pointer,
			"invalid type for integer legalization"
		);

		if (type == data_type::id::pointer) {
			*out_mask = 0;
			return qword;
		}

		x64_data_type t = none;
		const u8 bit_width = data_type.get_bit_width();
		i32 bits = 0;

		if (bit_width <= 8) {
			bits = 8;
			t = byte;
		}
		else if (bit_width <= 16) {
			bits = 16;
			t = word;
		}
		else if (bit_width <= 32) {
			bits = 32;
			t = dword;
		}
		else if (bit_width <= 64) {
			bits = 64;
			t = qword;
		}

		ASSERT(bits != 0, "TODO: large int support");
		const u64 mask = ~UINT64_C(0) >> (64 - bit_width);
		*out_mask = (bit_width == bits) ? 0 : mask;
		return t;
	}

	std::array<instruction::description, INSTRUCTION_COUNT> initialize_x64_instruction_table() {
		std::array<instruction::description, INSTRUCTION_COUNT> table = {};

#define X(a, b, c, ...) table[static_cast<i32>(instruction::type:: ## a)] = \
		{ .mnemonic = b, .cat = c, __VA_ARGS__}

		X(RET, "ret", instruction::category::byte, .op = 0xC3);
		X(INT3, "int3", instruction::category::byte, .op = 0xCC);
		X(STOSB, "rep stosb", instruction::category::byte, .op = 0xAA);
		X(MOVSB, "rep movsb", instruction::category::byte, .op = 0xA4);
		X(CAST, "cvt", instruction::category::byte, .op = 0x99);
		X(system_call, "syscall", instruction::category::byte_ext, .op = 0x05);
		X(RDTSC, "rdtsc", instruction::category::byte_ext, .op = 0x31);
		X(UD2, "ud2", instruction::category::byte_ext, .op = 0x0B);

		X(NOT, "not", instruction::category::unary, .op_i = 0xF7, .rx_i = 0x02);
		X(NEG, "neg", instruction::category::unary, .op_i = 0xF7, .rx_i = 0x03);
		X(MUL, "mul", instruction::category::unary, .op_i = 0xF7, .rx_i = 0x04);
		X(div, "div", instruction::category::unary, .op_i = 0xF7, .rx_i = 0x06);
		X(integral_div, "idiv", instruction::category::unary, .op_i = 0xF7, .rx_i = 0x07);

		X(call, "call", instruction::category::unary, .op = 0xE8, .op_i = 0xFF, .rx_i = 0x02);
		X(jmp, "jmp", instruction::category::unary, .op = 0xE9, .op_i = 0xFF, .rx_i = 0x04);

		// jcc
		X(JO, "jo", instruction::category::unary_ext, .op = 0x80);
		X(JNO, "jno", instruction::category::unary_ext, .op = 0x81);
		X(JB, "jb", instruction::category::unary_ext, .op = 0x82);
		X(JNB, "jnb", instruction::category::unary_ext, .op = 0x83);
		X(JE, "je", instruction::category::unary_ext, .op = 0x84);
		X(JNE, "jne", instruction::category::unary_ext, .op = 0x85);
		X(JBE, "jbe", instruction::category::unary_ext, .op = 0x86);
		X(JA, "ja", instruction::category::unary_ext, .op = 0x87);
		X(JS, "js", instruction::category::unary_ext, .op = 0x88);
		X(JNS, "jns", instruction::category::unary_ext, .op = 0x89);
		X(JP, "jp", instruction::category::unary_ext, .op = 0x8A);
		X(JNP, "jnp", instruction::category::unary_ext, .op = 0x8B);
		X(JL, "jl", instruction::category::unary_ext, .op = 0x8C);
		X(JGE, "jge", instruction::category::unary_ext, .op = 0x8D);
		X(JLE, "jle", instruction::category::unary_ext, .op = 0x8E);
		X(JG, "jg", instruction::category::unary_ext, .op = 0x8F);

		// setcc
		X(SETO, "seto", instruction::category::unary_ext, .op = 0x90);
		X(SETNO, "setno", instruction::category::unary_ext, .op = 0x91);
		X(SETB, "setb", instruction::category::unary_ext, .op = 0x92);
		X(SETNB, "setnb", instruction::category::unary_ext, .op = 0x93);
		X(SETE, "sete", instruction::category::unary_ext, .op = 0x94);
		X(SETNE, "setne", instruction::category::unary_ext, .op = 0x95);
		X(SETBE, "setbe", instruction::category::unary_ext, .op = 0x96);
		X(SETA, "seta", instruction::category::unary_ext, .op = 0x97);
		X(SETS, "sets", instruction::category::unary_ext, .op = 0x98);
		X(SETNS, "setns", instruction::category::unary_ext, .op = 0x99);
		X(SETP, "setp", instruction::category::unary_ext, .op = 0x9A);
		X(SETNP, "setnp", instruction::category::unary_ext, .op = 0x9B);
		X(SETL, "setl", instruction::category::unary_ext, .op = 0x9C);
		X(SETGE, "setge", instruction::category::unary_ext, .op = 0x9D);
		X(SETLE, "setle", instruction::category::unary_ext, .op = 0x9E);
		X(SETG, "setg", instruction::category::unary_ext, .op = 0x9F);

		// cmovcc
		X(CMOVO, "cmovo", instruction::category::binop_ext, .op = 0x40);
		X(CMOVNO, "cmovno", instruction::category::binop_ext, .op = 0x41);
		X(CMOVB, "cmovb", instruction::category::binop_ext, .op = 0x42);
		X(CMOVNB, "cmovnb", instruction::category::binop_ext, .op = 0x43);
		X(CMOVE, "cmove", instruction::category::binop_ext, .op = 0x44);
		X(CMOVNE, "cmovne", instruction::category::binop_ext, .op = 0x45);
		X(CMOVBE, "cmovbe", instruction::category::binop_ext, .op = 0x46);
		X(CMOVA, "cmova", instruction::category::binop_ext, .op = 0x47);
		X(CMOVS, "cmovs", instruction::category::binop_ext, .op = 0x48);
		X(CMOVNS, "cmovns", instruction::category::binop_ext, .op = 0x49);
		X(CMOVP, "cmovp", instruction::category::binop_ext, .op = 0x4A);
		X(CMOVNP, "cmovnp", instruction::category::binop_ext, .op = 0x4B);
		X(CMOVL, "cmovl", instruction::category::binop_ext, .op = 0x4C);
		X(CMOVGE, "cmovge", instruction::category::binop_ext, .op = 0x4D);
		X(CMOVLE, "cmovle", instruction::category::binop_ext, .op = 0x4E);
		X(CMOVG, "cmovg", instruction::category::binop_ext, .op = 0x4F);

		// binary ops but they have an implicit CL on the right-hand side
		X(SHL, "shl", instruction::category::binop_cl, .op = 0xD2, .op_i = 0xC0, .rx_i = 0x04);
		X(SHR, "shr", instruction::category::binop_cl, .op = 0xD2, .op_i = 0xC0, .rx_i = 0x05);
		X(ROL, "rol", instruction::category::binop_cl, .op = 0xD2, .op_i = 0xC0, .rx_i = 0x00);
		X(ROR, "ror", instruction::category::binop_cl, .op = 0xD2, .op_i = 0xC0, .rx_i = 0x01);
		X(SAR, "sar", instruction::category::binop_cl, .op = 0xD2, .op_i = 0xC0, .rx_i = 0x07);

		X(ADD, "add", instruction::category::binop, .op = 0x00, .op_i = 0x80, .rx_i = 0x00);
		X(OR, "or", instruction::category::binop, .op = 0x08, .op_i = 0x80, .rx_i = 0x01);
		X(AND, "and", instruction::category::binop, .op = 0x20, .op_i = 0x80, .rx_i = 0x04);
		X(SUB, "sub", instruction::category::binop, .op = 0x28, .op_i = 0x80, .rx_i = 0x05);
		X(XOR, "xor", instruction::category::binop, .op = 0x30, .op_i = 0x80, .rx_i = 0x06);
		X(CMP, "cmp", instruction::category::binop, .op = 0x38, .op_i = 0x80, .rx_i = 0x07);
		X(mov, "mov", instruction::category::binop, .op = 0x88, .op_i = 0xC6, .rx_i = 0x00);
		X(TEST, "test", instruction::category::binop, .op = 0x84, .op_i = 0xF6, .rx_i = 0x00);

		// misc integer ops
		X(movabs, "mov", instruction::category::binop_plus, .op = 0xB8);
		X(XCHG, "xchg", instruction::category::binop, .op = 0x86);
		X(lea, "lea", instruction::category::binop, .op = 0x8D);
		X(XADD, "xadd", instruction::category::binop_ext, .op = 0xC0);
		X(integral_multiplication, "imul", instruction::category::binop_ext, .op = 0xAF);
		X(IMUL3, "imul", instruction::category::binop, .op = 0x69);
		X(MOVSXB, "movsxb", instruction::category::binop_ext2, .op = 0xBE);
		X(MOVSXW, "movsxw", instruction::category::binop_ext2, .op = 0xBF);
		X(MOVSXD, "movsxd", instruction::category::binop, .op = 0x63);
		X(MOVZXB, "movzxb", instruction::category::binop_ext2, .op = 0xB6);
		X(MOVZXW, "movzxw", instruction::category::binop_ext2, .op = 0xB7);

		// gpr<->xmm
		X(MOV_I2F, "mov", instruction::category::binop_ext3, .op = 0x6E);
		X(MOV_F2I, "mov", instruction::category::binop_ext3, .op = 0x7E);

		// SSE binops
		X(floating_point_mov, "mov", instruction::category::binop_sse, .op = 0x10);
		X(FP_ADD, "add", instruction::category::binop_sse, .op = 0x58);
		X(FP_MUL, "mul", instruction::category::binop_sse, .op = 0x59);
		X(FP_SUB, "sub", instruction::category::binop_sse, .op = 0x5C);
		X(FP_MIN, "min", instruction::category::binop_sse, .op = 0x5D);
		X(FP_DIV, "div", instruction::category::binop_sse, .op = 0x5E);
		X(FP_MAX, "max", instruction::category::binop_sse, .op = 0x5F);
		X(FP_CMP, "cmp", instruction::category::binop_sse, .op = 0xC2);
		X(FP_UCOMI, "ucomi", instruction::category::binop_sse, .op = 0x2E);
		X(FP_CVT32, "cvtsi", instruction::category::binop_sse, .op = 0x2A);
		X(FP_CVT64, "cvtsi", instruction::category::binop_sse, .op = 0x2A);
		X(FP_CVT, "cvt", instruction::category::binop_sse, .op = 0x5A);
		X(FP_CVTT, "rsqrt", instruction::category::binop_sse, .op = 0x2C);
		X(FP_SQRT, "and", instruction::category::binop_sse, .op = 0x51);
		X(FP_RSQRT, "or", instruction::category::binop_sse, .op = 0x52);
		X(FP_AND, "xor", instruction::category::binop_sse, .op = 0x54);
		X(FP_OR, "or", instruction::category::binop_sse, .op = 0x56);
		X(FP_XOR, "xor", instruction::category::binop_sse, .op = 0x57);

		#undef X

		return table;
	}

	handle<value> value::create_label(handle<node> target) {
		const handle<value> val = create_value<target_prop>();
		val->m_type = label;
		val->get<target_prop>()->target = target;
		return val;
	}

	handle<value> value::create_imm(i32 imm) {
		const handle<value> val = create_value<empty_property>();
		val->m_type = type::imm;
		val->m_imm = imm;
		return val;
	}

	value::type value::get_type() const {
		return m_type;
	}

	i8 value::get_reg() const {
		return m_reg;
	}

	i8 value::get_index() const {
		return m_index;
	}

	i32 value::get_imm() const {
		return m_imm;
	}

	scale value::get_scale() const {
		return m_scale;
	}

	void value::set_type(type type) {
		m_type = type;
	}

	void value::set_reg(i8 reg) {
		m_reg = reg;
	}

	void value::set_index(i8 index) {
		m_index = index;
	}

	void value::set_scale(scale scale) {
		m_scale = scale;
	}

	void value::set_imm(i32 imm) {
		m_imm = imm;
	}

	bool value::matches(handle<value> b) const {
		if (m_type != b->m_type) {
			return false;
		}

		if (m_type == mem) {
			return
				m_reg == b->m_reg && 
				m_index == b->m_index &&
				m_scale== b->m_scale;
		}

		return (m_type == gpr || m_type == xmm) ? m_reg == b->m_reg : false;
	}
}
