#pragma once
#include "intermediate_representation/target/arch/architecture.h"

namespace ir {
	namespace x64 {
		enum class x64_segment {
			default_seg = 0,
			es, cs, ss, ds, gs, fs,
		};

		enum x64_flags : u16 {
			// uses xmm registers for the reg array
			x64_flags_xmm_reg = 1u << 0u,

			// r/m is a memory operand
			x64_flags_use_mem_op = 1u << 1u,

			// r/m is a rip-relative address (x64_flags_use_mem_op is always set when this is set)
			x64_flags_use_rip_mem = 1u << 2u,

			// LOCK prefix is present
			x64_flags_lock = 1u << 3u,

			// uses a signed immediate
			x64_flags_immediate = 1u << 4u,

			// absolute means it's using the 64bit immediate (cannot be applied while a memory operand is active)
			x64_flags_absolute = 1u << 5u,

			// set if the r/m can be found on the right hand side
			x64_flags_direction = 1u << 6u,

			// uses the second data type because the instruction is weird like MOVSX or MOVZX
			x64_flags_two_data_types = 1u << 7u,

			// REP prefix is present
			x64_flags_rep = 1u << 8u,

			// REPNE prefix is present
			x64_flags_repne = 1u << 9u,
		};

		// declare as a flag enum
		FLAG_ENUM(x64_flags);

		enum op_type {
			OP_8BIT = 1,
			OP_64BIT = 2,
			OP_FAKERX = 4,
			OP_2DT = 8,
			OP_SSE = 16,
		};

		enum op_encoding {
			OP_BAD = 0x0000,
			OP_MR = 0x1000,
			OP_RM = 0x2000,
			OP_M = 0x3000,
			OP_MI = 0x4000,
			OP_MI8 = 0x5000,
			OP_MC = 0x6000,
			OP_PLUSR = 0x7000,
			OP_0ARY = 0x8000,
			OP_REL8 = 0x9000,
			OP_REL32 = 0xA000,
			OP_IMM = 0xB000,
		};

		struct x64_instruction {
			i32 opcode;
			reg registers[4];
			x64_flags flags;

			i32 data_type_1 : 8;
			i32 data_type_2 : 8;
			x64_segment segment;
			u8 length : 4;

			u8 base;
			mem memory;

			union {
				i32 imm;
				u64 abs;
			};

			std::string get_mnemonic() const;
		};
	}

	class x64_disassembler : public disassembler {
	public:
		auto disassemble(
			const utility::byte_buffer& bytecode, const codegen_context& context
		) -> utility::string override;
	private:
		static handle<symbol_patch> disassemble_block(
			const utility::byte_buffer& bytecode,
			const codegen_context& context,
			handle<symbol_patch> patch,
			u64 basic_block, 
			utility::range<u64> range,
			utility::string& assembly
		);

		static bool disassemble_instruction(
			const utility::byte_buffer& bytecode,
			x64::x64_instruction& inst
		);

		static ptr_diff disassemble_memory_operand(
			const utility::byte_buffer& bytecode,
			u64 length,
			i32 reg_slot,
			u8 mod,
			u8 rm,
			u8 rex,
			x64::x64_instruction& inst
		);

		static u64 emit_get_label(const codegen_context& context, u64 position);

		static std::array<u16, 256> get_first_table();
		static std::array<u16, 256> get_ext_table();
	};
}
