#pragma once
#include "intermediate_representation/codegen/instruction.h"

namespace sigma::ir {
	struct codegen_context;

	struct global_operand {
		
	};

	/**
	 * \brief Instruction operand, automatically compares down to its own underlying type.
	 */
	struct instruction_operand : utility::property<absolute, label, handle<symbol>, global_operand> {
		struct type {
			enum underlying {
				NONE,
				FLAGS,
				GPR,
				XMM,
				IMM,
				MEM,
				GLOBAL,
				ABS,
				LABEL
			};

			type();
			type(underlying type);

			[[nodiscard]] auto to_string() const -> std::string;

			operator underlying() const;
		private:
			underlying m_type;
		};

		static auto create_label(const codegen_context& context, u64 target) -> handle<instruction_operand>;
		static auto create_global(const codegen_context& context, handle<symbol> symbol, i32 disp) -> handle<instruction_operand>;
		static auto create_imm(const codegen_context& context,i32 imm) -> handle<instruction_operand>;
		static auto create_abs(const codegen_context& context,u64 abs) -> handle<instruction_operand>;

		auto matches(handle<instruction_operand> b) const -> bool;

		void set_type(type type);
		auto get_type() const -> type;

		u8 reg;
		u8 index;
		memory_scale scale;

		i32 immediate; // TODO: investigate whether we actually need this
	private:
		type m_type;
	};

	bool operator==(handle<instruction_operand> operand, instruction_operand::type::underlying type);
} // namespace sigma::ir
