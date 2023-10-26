#pragma once
#include "intermediate_representation/nodes/node.h"

namespace ir::cg {
	class phi_value {
	public:
		phi_value(handle<node> n, handle<node> phi, i32 source = -1, i32 destination = -1);

		void set_node(handle<node> node);
		void set_phi(handle<node> phi);
		void set_source(i32 source);
		void set_destination(i32 dst);

		handle<node> get_phi() const;
		handle<node> get_node() const;
		i32 get_destination() const;
	private:
		handle<node> m_node;
		handle<node> m_phi;
		i32 m_source;
		i32 m_destination;
	};

	struct immediate_prop {
		i32 value;
	};

	struct absolute_prop {
		u64 value;
	};

	struct node_prop {
		handle<node> value;
	};

	class instruction : public utility::property<
		immediate_prop,
		absolute_prop,
		node_prop
	>{
	public:
		enum category : u8 {
			// nullary
			byte,
			byte_ext,

			// unary
			unary,
			unary_ext,  // 0F

			// binary
			binop,
			binop_plus, // +r
			binop_ext,  // 0F
			binop_ext2, // 0F (movzx, movsx)
			binop_ext3, // 66 (movd, movq)
			binop_cl,   // implicit CL, for shift operations

			// sse
			binop_sse
		};

		enum type {
			RET,
			INT3,
			STOSB,
			MOVSB,
			CAST,
			system_call,
			RDTSC,
			UD2,
			NOT,
			NEG,
			MUL,
			div,
			integral_div,
			call,
			jmp,

			// jcc
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
			mov,
			TEST,

			// misc integer ops
			movabs,
			XCHG,
			lea,
			XADD,
			integral_multiplication,
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
			floating_point_mov,
			FP_ADD,
			FP_MUL,
			FP_SUB,
			FP_MIN,
			FP_DIV,
			FP_MAX,
			FP_CMP,
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

			label = 1024,
			line,
			inl, // inline
			terminator,
			epilogue,
			entry,
			zero
		};

		enum flags {
			none = 0,
			lock = 1,
			rep = 2,
			repne = 4,

			mem = 16,
			global = 32,
			node = 64,
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
			u8 op, op_i, rx_i;
		};

		instruction() = default;

		void set_type(type type);
		void set_flags(flags flags);
		void set_memory_slot(i32 slot);
		void set_disp(i32 disp);
		void set_scale(u8 scale);
		void set_next_instruction(handle<instruction> next);
		void set_operands(const utility::slice<i32>& operands);
		void set_operand(u64 index, i32 value);
		void set_data_type(i32 data_type);
		void set_in_count(u8 in_count);
		void set_out_count(u8 out_count);
		void set_tmp_count(u8 tmp_count);
		void set_time(i32 time);

		handle<instruction> get_next_instruction() const;
		utility::slice<i32>& get_operands();
		i32 get_operand(u64 index) const;
		const utility::slice<i32>& get_operands() const;
		u8 get_in_count() const;
		u8 get_out_count() const;
		u8 get_tmp_count() const;
		u16 get_operand_count() const;
		flags get_flags() const;
		type get_type() const;
		i32 get_time() const;
		i32 get_data_type() const;
		u8 get_scale() const;
		i32 get_disp() const;
		i32 get_memory_slot() const;

		bool is_terminator() const;
	private:
		type m_type = zero;
		flags m_flags = none;
		u8 m_in_count = 0;
		u8 m_out_count = 0;
		u8 m_tmp_count = 0;
		i32 m_memory_slot;
		i32 m_disp;
		i32 m_time = 0;
		u8 m_scale;
		i32 m_data_type = 0;            // x64_data_type 
		handle<instruction> m_next;
		utility::slice<i32> m_operands;
	};

	// declare as a flag enum
	FLAG_ENUM(instruction::flags);
}

