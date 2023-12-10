#pragma once
#include "intermediate_representation/target/arch/architecture.h"
#include "intermediate_representation/codegen/memory/memory.h"

namespace sigma::ir {
	namespace x64 {
		enum data_type {
			NONE = 0,

			BYTE,   WORD,   DWORD,  QWORD,
			PBYTE,  PWORD,  PDWORD, PQWORD,
			SSE_SS, SSE_SD, SSE_PS, SSE_PD,

			XMMWORD
		};

		struct register_class {
			enum value : u8 {
				GPR, 
				XMM,

				FIRST_GPR = 0,
				FIRST_XMM = 16
			};
		};

		enum gpr : u8 {
			RAX, RCX, RDX, RBX, RSP, RBP, RSI, RDI,
			R8,  R9,  R10, R11, R12, R13, R14, R15,
		};

		enum xmm : u8 {
			XMM0, XMM1, XMM2,  XMM3,  XMM4,  XMM5,  XMM6,  XMM7,
			XMM8, XMM9, XMM10, XMM11, XMM12, XMM13, XMM14, XMM15
		};

		inline auto get_register_name(reg reg, i32 dt) -> std::string {
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

		inline auto get_type_name(i32 dt) -> std::string {
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

		enum mod : u8 {
			INDIRECT = 0,                 // [rax]
			INDIRECT_DISPLACEMENT_8 = 1,  // [rax + disp8]
			INDIRECT_DISPLACEMENT_32 = 2, // [rax + disp32]
			DIRECT = 3,                   // rax
		};

		enum conditional {
			O, NO, B, NB, E, NE, BE, A,
			S, NS, P, NP, L, GE, LE, G
		};
	}
	
	static auto rex(bool is_64_bit, u8 rx, u8 base, u8 index) -> u8;
	static auto mod_rx_rm(x64::mod mod, u8 rx, u8 rm) -> u8;

	class x64_architecture : public architecture {
	public:
		auto get_register_intervals() -> std::vector<live_interval> override;
		void select_instructions(codegen_context& context) override;
		auto emit_bytecode(codegen_context& context) -> utility::byte_buffer override;
	private:
		#pragma region instruction selection
		void select_instructions_region(
			codegen_context& context, handle<node> block_entry, handle<node> block_end, u64 rpo_index
		);

		void select_instruction(
			codegen_context& context, handle<node> n, reg destination
		);

		auto select_memory_access_instruction(
			codegen_context& context, handle<node> n, reg destination, i32 store_op, i32 source
		) -> handle<instruction>;

		auto select_array_access_instruction(
			codegen_context& context, handle<node> n, reg destination, i32 store_op, i32 source
		) -> handle<instruction>;

		auto select_instruction_cmp(
			codegen_context& context, handle<node> n
		) -> x64::conditional;

		void dfs_schedule(
			codegen_context& context, handle<basic_block> bb, handle<node> n, bool is_end
		);

		void dfs_schedule_phi(
			codegen_context& context, handle<basic_block> bb, handle<node> phi, ptr_diff phi_index
		);

		template<typename extra_type = utility::empty_property>
		static auto create_instruction(
			codegen_context& context,
			instruction::instruction_type type,
			data_type data_type, 
			u8 out_count,
			u8 in_count, 
			u8 tmp_count
		) -> handle<instruction> {
			const handle<instruction> inst = context.create_instruction<extra_type>(
				out_count + in_count + tmp_count
			);

			inst->type = type;
			inst->data_type = legalize_data_type(data_type);
			inst->in_count = in_count;
			inst->out_count = out_count;
			inst->tmp_count = tmp_count;

			return inst;
		}

		auto allocate_virtual_register(
			codegen_context& context, handle<node> n, const data_type& data_type
		) -> reg;

		auto input_reg(codegen_context& context, handle<node> n) -> reg;

		auto create_label(
			codegen_context& context,
			handle<node> target
		) -> handle<instruction>;

		auto create_jump(
			codegen_context& context,
			u64 target
		) -> handle<instruction>;

		auto create_jcc(
			codegen_context& context, 
			int target, x64::conditional cc
		) -> handle<instruction>;

		auto create_move(
			codegen_context& context, 
			const data_type& data_type, 
			reg destination,
			reg source
		) -> handle<instruction>;

		auto create_mr(
			codegen_context& context,
			instruction::instruction_type type, 
			const data_type& data_type, 
			reg base,
			i32 index, 
			scale scale,
			i32 disp, 
			i32 source
		) -> handle<instruction>;

		auto create_rm(
			codegen_context& context,
			instruction::instruction_type type, 
			const data_type& data_type,
			reg destination,
			reg base, 
			i32 index, 
			scale scale, 
			i32 disp
		) -> handle<instruction>;

		auto create_rr(
			codegen_context& context,
			instruction::instruction_type type,
			const data_type& data_type, 
			reg destination, 
			reg source
		) -> handle<instruction>;

		auto create_rr_no_destination(
			codegen_context& context, 
			instruction::instruction_type type,
			const data_type& data_type, 
			reg left, 
			reg right
		) -> handle<instruction>;

		auto create_immediate(
			codegen_context& context,
			instruction::instruction_type type,
			const data_type& data_type,
			reg destination, 
			i32 value
		) -> handle<instruction>;

		auto create_zero(
			codegen_context& context, 
			const data_type& data_type, 
			reg destination
		) -> handle<instruction>;

		auto create_abs(
			codegen_context& context,
			instruction::instruction_type type,
			const data_type& data_type, 
			reg destination, 
			u64 immediate
		) -> handle<instruction>;

		auto create_ri(
			codegen_context& context,
			instruction::instruction_type type,
			const data_type& data_type, 
			reg src,
			i32 imm
		) -> handle<instruction>;

		auto create_rri(
			codegen_context& context,
			instruction::instruction_type type,
			const data_type& data_type, 
			reg destination, 
			reg source,
			i32 immediate_value
		) -> handle<instruction>;

		auto create_rrr(
			codegen_context& context,
			instruction::instruction_type type,
			const data_type& data_type,
			reg destination, 
			reg left, 
			reg right
		) -> handle<instruction>;

		auto create_rrm(
			codegen_context& context,
			instruction::instruction_type type,
			const data_type& data_type,
			reg destination, 
			reg source,
			reg base, 
			i32 index, 
			scale scale,
			i32 disp
		) -> handle<instruction>;

		auto create_op_global(
			codegen_context& context,
			instruction::instruction_type type, 
			const data_type& data_type,
			reg dst,
			handle<symbol> s
		) -> handle<instruction>;

		static auto try_for_imm32(handle<node> n, i32 bits, i32& out) -> bool;
		static auto can_folded_store(
			codegen_context& context, handle<node> memory, handle<node> address, handle<node> source
		) -> i32;

		static auto classify_register_class(
			const data_type& data_type
		) -> u8;

		static auto legalize_integer_data_type(
			u64* out_mask, const data_type& data_type
		) -> x64::data_type;

		static auto legalize_data_type(
			const data_type& data_type
		) -> i32;

		static void resolve_stack_usage(codegen_context& context);
		#pragma endregion
		#pragma region code generation
		static void emit_function_prologue(
			codegen_context& context, utility::byte_buffer& bytecode
		);

		static void emit_function_body(
			codegen_context& context, utility::byte_buffer& bytecode
		);

		static void emit_function_epilogue(
			const codegen_context& context, utility::byte_buffer& bytecode
		);

		static auto get_instruction_table() -> std::array<instruction::description, 120>;

		static void emit_instruction_0(
			instruction::instruction_type type, i32 data_type, utility::byte_buffer& bytecode
		);

		static void emit_instruction_1(
			codegen_context& context,
			instruction::instruction_type type, 
			handle<codegen_temporary> r, 
			i32 dt, 
			utility::byte_buffer& bytecode
		);

		static void emit_instruction_2(
			codegen_context& context, 
			instruction::instruction_type type,
			handle<codegen_temporary> a, 
			handle<codegen_temporary> b,
			i32 data_type, 
			utility::byte_buffer& bytecode
		);

		static void emit_memory_operand(
			codegen_context& context, u8 rx, handle<codegen_temporary> a, utility::byte_buffer& bytecode
		);
		static auto resolve_interval(
			const codegen_context& context, handle<instruction> inst, u8 i,handle<codegen_temporary> val
		) -> u8;

		static void emit_symbol_patch(
			codegen_context& context, handle<symbol> target, u64 pos
		);

		static inline std::array<instruction::description, 120> s_instruction_table = get_instruction_table();
		#pragma endregion
	};
} // namespace sigma::ir
