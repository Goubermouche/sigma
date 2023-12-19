#pragma once
#include "intermediate_representation/target/arch/architecture.h"

namespace sigma::ir {
	namespace x64 {
		enum class x64_segment {
			DEFAULT_SEGMENT = 0,
			ES, CS, SS, DS, GS, FS,
		};

		enum x64_flags : u16 {
			XMM_REG        = 1u << 0u, // uses xmm registers for the reg array
			USE_MEM_OP     = 1u << 1u, // r/m is a memory operand
			USE_RIP_MEM    = 1u << 2u, // r/m is a rip-relative address (x64_flags_use_mem_op is always set when this is set)
			LOCK           = 1u << 3u, // LOCK prefix is present
			IMMEDIATE      = 1u << 4u, // uses a signed immediate
			ABSOLUTE       = 1u << 5u, // absolute means it's using the 64bit immediate (cannot be applied while a memory operand is active)
			DIRECTION      = 1u << 6u, // set if the r/m can be found on the right hand side
			TWO_DATA_TYPES = 1u << 7u, // uses the second data type because the instruction is weird like MOVSX or MOVZX
			REP            = 1u << 8u, // REP prefix is present
			REPNE          = 1u << 9u, // REPNE prefix is present
		};

		// declare as a flag enum
		FLAG_ENUM(x64_flags);

		enum class op_type : u16 {
			OP_8BIT   = 1,
			OP_64BIT  = 2,
			OP_FAKERX = 4,
			OP_2DT    = 8,
			OP_SSE    = 16,
		};

		INTEGRAL_ENUM(op_type);

		enum class op_encoding : u16 {
			OP_BAD   = 0x0000,
			OP_MR    = 0x1000,
			OP_RM    = 0x2000,
			OP_M     = 0x3000,
			OP_MI    = 0x4000,
			OP_MI8   = 0x5000,
			OP_MC    = 0x6000,
			OP_PLUSR = 0x7000,
			OP_0ARY  = 0x8000,
			OP_REL8  = 0x9000,
			OP_REL32 = 0xA000,
			OP_IMM   = 0xB000,
		};

		INTEGRAL_ENUM(op_encoding);

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
} // namespace sigma::ir
