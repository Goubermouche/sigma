#include "x64_emitter.h"

namespace code_generator {
	x64_emitter::x64_emitter()
		: m_instructions(x64_instruction_list) {}

	std::vector<utility::byte> x64_emitter::emit_imm64(i64 value) {
		std::vector<utility::byte> bytes(8);

		for (int i = 0; i < 8; ++i) {
			bytes[i] = static_cast<uint8_t>(value & 0xFF);
			value >>= 8;
		}

		return bytes;
	}

	void x64_instruction_visitor::operator()(
		ir::stack_allocation_instruction_ptr instruction
	) const {

	}
}