#pragma once
#include "intermediate_representation/code_generation/live_interval.h"

// Core instruction selection file. Every instruction is stored withing a machine
// basic block (see intermediate_representation/code_generation/machine_block.h for
// more information). Each instruction holds several inputs and has several flags,
// a category, and a general instruction type.

namespace ir::cg {
	struct code_generator_context;

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

		void set_operands(const utility::slice<i32>& operands);
		void set_next_instruction(handle<instruction> next);
		void set_displacement(i32 displacement);
		void set_operand(u64 index, i32 value);
		void set_data_type(i32 data_type);
		void set_out_count(u8 out_count);
		void set_tmp_count(u8 tmp_count);
		void set_memory_slot(i32 slot);
		void set_in_count(u8 in_count);
		void set_flags(flags flags);
		void set_scale(scale scale);
		void set_type(type type);
		void set_time(u64 time);

		handle<instruction> get_next_instruction() const;
		const utility::slice<i32>& get_operands() const;
		utility::slice<i32>& get_operands();
		i32 get_operand(u64 index) const;
		u16 get_operand_count() const;
		i32 get_displacement() const;
		i32 get_memory_slot() const;
		bool is_terminator() const;
		i32 get_data_type() const;
		u8 get_out_count() const;
		u8 get_tmp_count() const;
		u8 get_in_count() const;
		flags get_flags() const;
		scale get_scale() const;
		type get_type() const;
		u64 get_time() const;

		/**
		 * \brief Creates a label instruction.
		 * \param context Code generation context
		 * \param target Associated block node
		 * \return Handle to the newly created instruction.
		 */
		static handle<instruction> create_label(
			code_generator_context& context, 
			handle<node> target
		);

		/**
		 * \brief Creates a jump (JMP) instruction.
		 * \param context Code generation context
		 * \param target Target block the instruction should jump to
		 * \return Handle to the newly created instruction.
		 */
		static handle<instruction> create_jump(
			code_generator_context& context,
			handle<node> target
		);

		/**
		 * \brief Creates a register-register-register (RRR) instruction.
		 * \param context Code generation context
		 * \param type The type of the instruction to be created
		 * \param data_type Data type associated with the operation
		 * \param destination Destination register
		 * \param left Left register
		 * \param right Right register
		 * \return Handle to the newly created instruction.
		 */
		static handle<instruction> create_rrr(
			code_generator_context& context,
			instruction::type type,
			const data_type& data_type,
			reg destination,
			reg left,
			reg right
		);

		/**
		 * \brief Creates a register-register-immediate (RRI) instruction.
		 * \param context Code generation context
		 * \param type The type of the instruction to be created
		 * \param data_type Data type associated with the operation
		 * \param destination Destination register
		 * \param source Source register
		 * \param imm The immediate value to be used
		 * \return Handle to the newly created instruction.
		 */
		static handle<instruction> create_rri(
			code_generator_context& context,
			instruction::type type,
			const data_type& data_type,
			reg destination,
			reg source,
			i32 immediate
		);

		/**
		 * \brief Creates a register-register-memory (RRM) instruction.
		 * \param context Code generation context
		 * \param type The type of the instruction to be created
		 * \param data_type Data type associated with the operation
		 * \param destination Destination register of the instruction
		 * \param source Source register of the instruction
		 * \param base Base register for the memory operand
		 * \param memory Memory operand
		 * \return Handle to the newly created instruction.
		 */
		static handle<instruction> create_rrm(
			code_generator_context& context,
			instruction::type type,
			const data_type& data_type,
			reg destination,
			reg source,
			reg base,
			mem memory
		);

		/**
		 * \brief Creates a register-memory (RM) instruction.
		 * \param context Code generation context
		 * \param type The type of the instruction to be created
		 * \param data_type Data type associated with the operation
		 * \param destination Destination register of the instruction
		 * \param base Base register for the memory operand
		 * \param memory Memory operand
		 * \return Handle to the newly created instruction.
		 */
		static handle<instruction> create_rm(
			code_generator_context& context,
			instruction::type type,
			const data_type& data_type,
			reg destination,
			reg base,
			mem memory
		);

		/**
		 * \brief Creates a memory-register (MR) instruction.
		 * \param context Code generation context
		 * \param type The type of the instruction to be created
		 * \param data_type Data type associated with the operation
		 * \param base Base register for the memory operand
		 * \param memory Memory operand
		 * \param source Source register of the instruction
		 * \return Handle to the newly created instruction.
		 */
		static handle<instruction> create_mr(
			code_generator_context& context,
			instruction::type type,
			const data_type& data_type,
			reg base,
			mem memory,
			i32 source
		);

		/**
		 * \brief Creates a mov instruction.
		 * \param context Code generation context
		 * \param data_type Data type associated with the operation
		 * \param destination Destination register ID
		 * \param source Source register ID
		 * \return Handle to the newly created instruction.
		 */
		static handle<instruction> create_move(
			code_generator_context& context,
			const data_type& data_type,
			reg destination,
			reg source
		);

		/**
		 * \brief Creates an absolute immediate value.
		 * \param context Code generation context
		 * \param type The type of the instruction to be created
		 * \param data_type Data type associated with the operation
		 * \param destination Destination register
		 * \param imm The immediate value to be used
		 * \return Handle to the newly created instruction.
		 */
		static handle<instruction> create_abs(
			code_generator_context& context,
			instruction::type type,
			const data_type& data_type,
			reg destination,
			u64 immediate
		);

		/**
		 * \brief Creates an instruction which sets the operand to0
		 * \param context Code generation context
		 * \param data_type Data type associated with the operation
		 * \param destination Destination register
		 * \return Handle to the newly created instruction.
		 */
		static handle<instruction> create_zero(
			code_generator_context& context,
			const data_type& data_type,
			reg destination
		);

		/**
		 * \brief Creates a 32-bit immediate instruction.
		 * \param context Code generation context
		 * \param type The type of the instruction to be created
		 * \param data_type Data type associated with the operation
		 * \param destination Destination register
		 * \param immediate The immediate value to be used
		 * \return Handle to the newly created instruction.
		 */
		static handle<instruction> create_immediate(
			code_generator_context& context,
			instruction::type type,
			const data_type& data_type,
			reg destination,
			i32 immediate
		);
	private:
		utility::slice<i32> m_operands; // TODO: storing operands as u8's? 
		handle<instruction> m_next;     // TODO: replace by a linked list
		flags m_flags = none;           // instruction flags
		type m_type = zero;             // instruction type

		// output, input, and temp counts of the instruction operands
		u8 m_out_count = 0;
		u8 m_tmp_count = 0;
		u8 m_in_count = 0;

		// TODO: comments 
		i32 m_data_type = 0;
		i32 m_displacement;
		i32 m_memory_slot;
		u64 m_time = 0;
		scale m_scale;
	};

	// declare as a flag enum
	FLAG_ENUM(instruction::flags);
}
