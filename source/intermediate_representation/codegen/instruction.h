#pragma once
#include "intermediate_representation/codegen/live_interval.h"

// Core instruction selection file. Every instruction is stored withing a machine
// basic block (see intermediate_representation/code_generation/machine_block.h for
// more information). Each instruction holds several inputs and has several flags,
// a category, and a general instruction type.

namespace ir {
	struct code_generator_context;

	struct immediate {
		i32 value;
	};

	struct absolute {
		u64 value;
	};

	struct label {
		u64 value;
	};

	struct instruction : utility::property<handle<symbol>, handle<node>, immediate, absolute, label>{
		enum class category : u8 {
			// nullary
			BYTE,
			BYTE_EXT,

			// unary
			UNARY,
			UNARY_EXT,  // 0F

			// binary
			BINOP,
			BINOP_PLUS, // +r
			BINOP_EXT_1,  // 0F
			BINOP_EXT_2, // 0F (movzx, movsx)
			BINOP_EXT_3, // 66 (movd, movq)
			BINOP_CL,   // implicit CL, for shift operations

			// sse
			BINOP_SSE
		};

		enum instruction_type {
			RET, INT3, STOSB, MOVSB, CAST, system_call, RDTSC, UD2,
			NOT, NEG, MUL, DIV, IDIV, CALL, JMP,

			// prefetching
			PREFETCHNTA, PREFETCH0, PREFETCH1, PREFETCH2,

			// jcc/**/
			JO,  JNO, JB,  JNB, JE, JNE, JBE, JA,
			JS,  JNS, JP,  JNP, JL, JGE, JLE, JG,

			// setcc
			SETO, SETNO, SETB, SETNB, SETE, SETNE, SETBE, SETA,
			SETS, SETNS, SETP, SETNP, SETL, SETGE, SETLE, SETG,

			// cmovcc
			CMOVO, CMOVNO, CMOVB, CMOVNB, CMOVE, CMOVNE, CMOVBE, CMOVA,
			CMOVS, CMOVNS, CMOVP, CMOVNP, CMOVL, CMOVGE, CMOVLE, CMOVG,

			// bitmagic
			BSF, BSR,

			// binary ops but they have an implicit CL on the right-hand side
			SHL, SHR, ROL, ROR, SAR, ADD, OR, AND, SUB, XOR, CMP, MOV, TEST,

			// misc integer ops
			MOVABS, XCHG, LEA, XADD, ÏMUL, IMUL3,
			MOVSXB, MOVSXW, MOVSXD, MOVZXB, MOVZXW,

			// gpr<->xmm
			MOV_I2F, MOV_F2I,

			// SSE binops
			FP_MOV,  FP_ADD,   FP_MUL,   FP_SUB,   FP_MIN,   FP_DIV, FP_MAX,
			FP_CMP,  P_CMP,    FP_UCOMI, FP_CVT32, FP_CVT64, FP_CVT, FP_CVTT,
			FP_SQRT, FP_RSQRT, FP_AND,   FP_OR,    FP_XOR,

			LABEL = 1024,
			LINE,
			INLINE,
			TERMINATOR,
			EPILOGUE,
			ENTRY,
			ZERO
		};

		enum instruction_flags {
			none = 0,
			lock = 1,
			rep = 2,
			repne = 4,

			mem_f = 16,
			global = 32,
			node_f = 64,
			attribute = 128,
			immediate = 256,
			absolute = 512,

			// memory op
			indexed = 1024,
			spill = 2048
		};

		struct description {
			std::string mnemonic;
			category cat;
			u8 op;
			u8 op_i;
			u8 rx_i;
		};

		instruction() = default;
		auto is_terminator() const -> bool;

		handle<instruction> next_instruction;
		u64 time = 0;

		instruction_flags flags = none; // instruction flags
		instruction_type  type  = ZERO; // instruction type
		i32 data_type = 0;              // instruction data type

		// operands
		utility::slice<i32> operands;
		u8 out_count  = 0;
		u8 tmp_count  = 0;
		u8 in_count   = 0;
		u8 save_count = 0;

		i32 displacement;
		u8 memory_slot;
		scale sc;
	};

	// declare as a flag enum
	FLAG_ENUM(instruction::instruction_flags);
}
