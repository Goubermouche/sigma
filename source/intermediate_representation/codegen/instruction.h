#pragma once
#include "intermediate_representation/codegen/live_interval.h"

// Core instruction selection file. Every instruction is stored withing a machine
// basic block (see intermediate_representation/code_generation/machine_block.h for
// more information). Each instruction holds several inputs and has several flags,
// a category, and a general instruction type.

namespace sigma::ir {
	struct code_generator_context;

	struct immediate { i32 value; };
	struct absolute { u64 value; };
	struct label { u64 value; };

	using instruction_properties = utility::property<
		handle<symbol>, handle<node>, immediate, absolute, label
	>;

	/**
	 * \brief Generic instruction abstraction, each instruction has an instruction
	 * type (instruction::type), operands and a data type. The given data type is special
	 * depending on the active architecture.
	 */
	struct instruction : instruction_properties {
		struct type {
			enum underlying {
				RET,
				INT3,
				NOP,
				STOSB,
				MOVSB,
				CAST,
				SYS_CALL,
				RDTSC,
				UD2,
				NOT,
				NEG,
				MUL,
				DIV,
				IDIV,
				CALL,
				JMP,

				// prefetching
				PREFETCHNTA,
				PREFETCH0,
				PREFETCH1,
				PREFETCH2,

				// jcc/**/
				JO,
				JNO,
				JB,
				JNB,
				JE,
				JNE,
				JBE,
				JA,
				JS,
				JNS,
				JP,
				JNP,
				JL,
				JGE,
				JLE,
				JG,

				// setcc
				SETO,
				SETNO,
				SETB,
				SETNB,
				SETE,
				SETNE,
				SETBE,
				SETA,
				SETS,
				SETNS,
				SETP,
				SETNP,
				SETL,
				SETGE,
				SETLE,
				SETG,

				// cmovcc
				CMOVO,
				CMOVNO,
				CMOVB,
				CMOVNB,
				CMOVE,
				CMOVNE,
				CMOVBE,
				CMOVA,
				CMOVS,
				CMOVNS,
				CMOVP,
				CMOVNP,
				CMOVL,
				CMOVGE,
				CMOVLE,
				CMOVG,

				// bit magic
				BSF,
				BSR,

				// binary ops but they have an implicit CL on the right-hand side
				SHL,
				SHR,
				ROL,
				ROR,
				SAR,
				ADD,
				OR,
				AND,
				SUB,
				XOR,
				CMP,
				MOV,
				TEST,

				// misc integer ops
				MOVABS,
				XCHG,
				LEA,
				XADD,
				IMUL,
				IMUL3,
				MOVSXB,
				MOVSXW,
				MOVSXD,
				MOVZXB,
				MOVZXW,

				// gpr<->xmm
				MOV_I2F,
				MOV_F2I,

				// SSE binops
				FP_MOV,
				FP_ADD,
				FP_MUL,
				FP_SUB,
				FP_MIN,
				FP_DIV,
				FP_MAX,
				FP_CMP,
				P_CMP,
				FP_UCOMI,
				FP_CVT32,
				FP_CVT64,
				FP_CVT,
				FP_CVTT,
				FP_SQRT,
				FP_RSQRT,
				FP_AND,
				FP_OR,
				FP_XOR,

				LABEL = 1024,
				LINE,
				INLINE,
				TERMINATOR,
				EPILOGUE,
				ENTRY,
				ZERO
			};

			type();
			type(underlying type);

			operator underlying() const;
			auto to_string() const->std::string;
		private:
			underlying m_type;
		};

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

		enum instruction_flags {
			NONE = 0,
			LOCK = 1,
			REP = 2,
			REPNE = 4,

			MEM = 16,
			GLOBAL = 32,
			NODE = 64,
			ATTRIBUTE = 128,
			IMMEDIATE = 256,
			ABSOLUTE = 512,

			// memory op
			INDEXED = 1024,
			SPILL = 2048,

			RET = 4096
		};

		struct description {
			std::string mnemonic;
			category category;
			u8 op;
			u8 op_i;
			u8 rx_i;
		};

		instruction() = default;

		auto is_terminator() const -> bool;

		void set_type(type type);
		auto get_type() const -> type;

		// store instructions as an in place linked list, since all the instructions are stored
		// in contiguous blocks of memory we should have relatively low cache miss rates
		handle<instruction> next_instruction;

		instruction_flags flags = NONE; // instruction flags
		// generic instruction data type, this should be interpreted differently depending on the
		// architecture / execution environment
		i32 data_type = 0;              

		// instruction operands, stored in the following order:
		// -    out
		// -    in
		// -    tmp
		// -    save
		utility::memory_view<i32> operands;
		u8 out_count = 0;
		u8 tmp_count = 0;
		u8 in_count = 0;
		u8 save_count = 0;

		u64 time = 0; // arbitrary counter used to determine the order of instructions in some cases
		mem memory;
	private:
		type m_type = type::ZERO;
	};

	bool operator==(handle<instruction> inst, instruction::type::underlying type);

	// declare as a flag enum
	FLAG_ENUM(instruction::instruction_flags);
	INTEGRAL_ENUM(instruction::instruction_flags);
} // namespace sigma::ir
