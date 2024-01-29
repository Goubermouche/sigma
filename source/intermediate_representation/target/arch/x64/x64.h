#pragma once
#include "intermediate_representation/target/arch/architecture.h"
#include "intermediate_representation/codegen/memory/memory.h"

namespace sigma::ir {
	namespace x64 {
		enum data_type {
			NONE = 0,

			BYTE,
			WORD,
			DWORD,
			QWORD,
			PBYTE,
			PWORD,
			PDWORD,
			PQWORD,
			SSE_SS,
			SSE_SD,
			SSE_PS,
			SSE_PD,
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

		enum class gpr : u8 {
			RAX,
			RCX,
			RDX,
			RBX,
			RSP,
			RBP,
			RSI,
			RDI,
			R8,
			R9,
			R10,
			R11,
			R12,
			R13,
			R14,
			R15,
		};

		INTEGRAL_ENUM(gpr);

		enum class xmm : u8 {
			XMM0,
			XMM1,
			XMM2,
			XMM3,
			XMM4,
			XMM5,
			XMM6,
			XMM7,
			XMM8,
			XMM9,
			XMM10,
			XMM11,
			XMM12,
			XMM13,
			XMM14,
			XMM15
		};

		INTEGRAL_ENUM(xmm);

		auto get_register_name(reg reg, i32 dt) -> std::string;
		auto get_type_name(i32 dt) -> std::string;

		enum mod : u8 {
			INDIRECT = 0,                 // [rax]
			INDIRECT_DISPLACEMENT_8 = 1,  // [rax + disp8]
			INDIRECT_DISPLACEMENT_32 = 2, // [rax + disp32]
			DIRECT = 3,                   // rax
		};

		enum class conditional {
			O, NO, B, NB, E, NE, BE, A,
			S, NS, P, NP, L, GE, LE, G
		};

		INTEGRAL_ENUM(conditional);
	}
	
	auto rex(bool is_64_bit, u8 rx, u8 base, u8 index) -> u8;
	auto mod_rx_rm(x64::mod mod, u8 rx, u8 rm) -> u8;

	class x64_architecture : public architecture {
	public:
		auto get_register_intervals() -> std::vector<live_interval> override;
		void select_instructions(codegen_context& context) override;
		auto emit_bytecode(codegen_context& context) -> utility::byte_buffer override;
	private:
		#pragma region instruction selection
		void select_instructions_region(codegen_context& context, handle<node> block_entry, handle<node> block_end, u64 rpo_index);
		void select_instruction(codegen_context& context, handle<node> n, reg destination);
		auto select_memory_access_instruction(codegen_context& context, handle<node> n, reg dst, i32 store_op, i32 src) -> handle<instruction>;
		auto select_array_access_instruction(codegen_context& context, handle<node> target, reg destination, i32 store_op, i32 source) -> handle<instruction>;
		auto select_instruction_cmp(codegen_context& context, handle<node> n) -> x64::conditional;

		void dfs_schedule(codegen_context& context, handle<basic_block> bb, handle<node> n, bool is_end);
		void dfs_schedule_phi(codegen_context& context, handle<basic_block> bb, handle<node> phi, ptr_diff phi_index);

		template<typename extra_type = utility::empty_property>
		static auto create_instruction(codegen_context& context, instruction::type type, data_type data_type, u8 out_count, u8 in_count, u8 tmp_count) -> handle<instruction> {
			const handle<instruction> instruction = context.create_instruction<extra_type>(out_count + in_count + tmp_count);

			instruction->set_type(type);
			instruction->data_type = legalize_data_type(data_type);
			instruction->in_count = in_count;
			instruction->out_count = out_count;
			instruction->tmp_count = tmp_count;

			return instruction;
		}

		auto allocate_virtual_register(codegen_context& context, handle<node> target, const data_type& data_type) -> reg;
		auto allocate_node_register(codegen_context& context, handle<node> target) -> reg;

		// specific instructions
		static auto create_label(codegen_context& context, handle<node> target) -> handle<instruction>;
		static auto create_jump(codegen_context& context, u64 target) -> handle<instruction>;
		static auto create_jcc(codegen_context& context, int target, x64::conditional cc) -> handle<instruction>;
		static auto create_move(codegen_context& context, const data_type& data_type, reg destination, reg source) -> handle<instruction>;

		// math constants
		static auto create_immediate(codegen_context& context, instruction::type type, const data_type& data_type, reg destination, i32 value) -> handle<instruction>;
		static auto create_abs(codegen_context& context, instruction::type type, const data_type& data_type, reg destination, u64 immediate) -> handle<instruction>;
		static auto create_zero(codegen_context& context, const data_type& data_type, reg destination) -> handle<instruction>;

		// other instructions
		static auto create_r(codegen_context& context, instruction::type type, const data_type& data_type, reg dst) -> handle<instruction>;
		static auto create_mr(codegen_context& context, instruction::type type, const data_type& data_type, reg base, i32 index, memory_scale scale, i32 disp, i32 source) -> handle<instruction>;
		static auto create_rm(codegen_context& context, instruction::type type, const data_type& data_type, reg destination, reg base, i32 index, memory_scale scale, i32 disp) -> handle<instruction>;
		static auto create_rr(codegen_context& context, instruction::type type, const data_type& data_type, reg destination, reg source) -> handle<instruction>;
		static auto create_rrd(codegen_context& context, instruction::type type,const data_type& data_type, reg destination, reg source) -> handle<instruction>;
		static auto create_ri(codegen_context& context, instruction::type type, const data_type& data_type, reg src, i32 imm) -> handle<instruction>;
		static auto create_rri(codegen_context& context, instruction::type type, const data_type& data_type, reg destination, reg source, i32 immediate_value) -> handle<instruction>;
		static auto create_rrr(codegen_context& context, instruction::type type, const data_type& data_type, reg destination, reg left, reg right) -> handle<instruction>;
		static auto create_rrm(codegen_context& context, instruction::type type, const data_type& data_type, reg destination, reg source, reg base, i32 index, memory_scale scale, i32 disp) -> handle<instruction>;
		static auto create_op_global(codegen_context& context, instruction::type type, const data_type& data_type, reg dst, handle<symbol> s) -> handle<instruction>;

		// data types
		static auto try_for_imm32(handle<node> n, i32 bits, i32& out) -> bool;
		static auto can_folded_store(codegen_context& context, handle<node> memory, handle<node> address, handle<node> source) -> i32;
		static auto classify_register_class(const data_type& data_type) -> u8;
		static auto legalize_integer_data_type(u64* out_mask, const data_type& data_type) -> x64::data_type;
		static auto legalize_data_type(const data_type& data_type) -> i32;

		#pragma endregion
		#pragma region code generation
		static void emit_nops_to_width(utility::byte_buffer& bytecode);
		static void emit_function_prologue(codegen_context& context, utility::byte_buffer& bytecode);
		static void emit_function_body(codegen_context& context, utility::byte_buffer& bytecode);
		static void emit_function_epilogue(const codegen_context& context, utility::byte_buffer& bytecode);

		// instruction types
		static void emit_instruction_0(instruction::type type, i32 data_type, utility::byte_buffer& bytecode);
		static void emit_instruction_1(codegen_context& context, instruction::type type, handle<instruction_operand> r, i32 dt, utility::byte_buffer& bytecode);
		static void emit_instruction_2(codegen_context& context, instruction::type type, handle<instruction_operand> a, handle<instruction_operand> b, i32 data_type, utility::byte_buffer& bytecode);

		static void emit_memory_operand(codegen_context& context, u8 rx, handle<instruction_operand> a, utility::byte_buffer& bytecode);
		static void emit_symbol_patch(codegen_context& context, handle<symbol> target, u64 pos);

		// misc
		static void resolve_stack_usage(codegen_context& context);
		static auto resolve_interval(const codegen_context& context, handle<instruction> inst, u8 i, handle<instruction_operand> val) -> u8;

		static auto get_instruction_table() -> std::array<instruction::description, 120>;

		static inline std::array<instruction::description, 120> s_instruction_table = get_instruction_table();
		#pragma endregion
	};
} // namespace sigma::ir
