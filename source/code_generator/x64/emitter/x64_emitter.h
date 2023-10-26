#pragma once
#include "code_generator/emitter/emitter.h"
#include "code_generator/emitter/x64/emitter/x64_instructions.h"

namespace code_generator {
	struct x64_instruction_visitor {
		void operator()(ir::stack_allocation_instruction_ptr instruction) const;
	};

	class x64_emitter : public emitter {
	public:
		x64_emitter();
	private:
		static std::vector<utility::byte> emit_imm64(i64 value);
	private:
		x64_instruction_table m_instructions;
	};
}
