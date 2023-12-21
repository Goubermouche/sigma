#include "instruction.h"
#include "intermediate_representation/target/arch/x64/x64.h"

namespace sigma::ir {
	auto instruction::is_terminator() const -> bool {
		return m_type == type::TERMINATOR || m_type == type::INT3 || m_type == type::UD2;
	}

	void instruction::set_type(type type) {
		m_type = type;
	}

	auto instruction::get_type() const -> instruction::type {
		return m_type;
  }

	instruction::type::type() : m_type(ZERO) {}

	instruction::type::type(underlying type) : m_type(type) {}

	instruction::type::operator underlying() const {
		return m_type;
	}

	auto instruction::type::to_string() const -> std::string {
		NOT_IMPLEMENTED();
		return "";
	}

	bool operator==(handle<instruction> inst, instruction::type::underlying type) {
		return inst->get_type() == type;
	}
} // namespace sigma::ir
