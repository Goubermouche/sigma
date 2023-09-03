#pragma once
#include "code_generator/emitter/emitter.h"
#include "code_generator/emitter/x64/x64_instructions.h"

namespace code_generator {
	class x64_emitter : public emitter {
	public:
		x64_emitter();

		std::vector<utility::byte> emit(
			const ir::builder& builder
		) override;
	private:
		static std::vector<utility::byte> emit_imm64(i64 value);
	private:
		x64_instruction_table m_instructions;
	};
}
