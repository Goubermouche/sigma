#include "x64_emitter.h"

namespace code_generator {
	x64_emitter::x64_emitter()
		: m_instructions(x64_instruction_list) {}

	std::vector<utility::byte> x64_emitter::emit(
		const ir::builder& builder
	) {
		std::vector<utility::byte> bytes;

		bytes.append_range(m_instructions.at(x64_operation::mov, x64_operand::rbx, x64_operand::r12));
		bytes.append_range(m_instructions.at(x64_operation::mov, x64_operand::rbx, x64_operand::r13));
		bytes.append_range(m_instructions.at(x64_operation::mov, x64_operand::rbx, x64_operand::r14));
		bytes.append_range(m_instructions.at(x64_operation::mov, x64_operand::rbx, x64_operand::rsi));
		bytes.append_range(m_instructions.at(x64_operation::mov, x64_operand::rsi, x64_operand::rax));

		bytes.append_range(m_instructions.at(x64_operation::mov, x64_operand::rbx, x64_operand::imm64));
		bytes.append_range(emit_imm64(1));

		bytes.append_range(m_instructions.at(x64_operation::mov, x64_operand::r12, x64_operand::imm64));
		bytes.append_range(emit_imm64(2));

		bytes.append_range(m_instructions.at(x64_operation::mov, x64_operand::r13, x64_operand::imm64));
		bytes.append_range(emit_imm64(3));

		bytes.append_range(m_instructions.at(x64_operation::mov, x64_operand::r14, x64_operand::imm64));
		bytes.append_range(emit_imm64(4));

		bytes.append_range(m_instructions.at(x64_operation::mov, x64_operand::r15, x64_operand::imm64));
		bytes.append_range(emit_imm64(5));

		// ret
		bytes.append_range(m_instructions.at(x64_operation::ret));

		return bytes;
	}

	std::vector<utility::byte> x64_emitter::emit_imm64(i64 value) {
		std::vector<utility::byte> bytes(8);

		for (int i = 0; i < 8; ++i) {
			bytes[i] = static_cast<uint8_t>(value & 0xFF);
			value >>= 8;
		}

		return bytes;
	}
}