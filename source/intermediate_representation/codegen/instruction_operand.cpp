#include "instruction_operand.h"
#include "intermediate_representation/codegen/codegen_context.h"

namespace sigma::ir {
	auto instruction_operand::matches(handle<instruction_operand> b) const -> bool {
		if (m_type != b->m_type) {
			return false;
		}

		if (m_type == type::MEM) {
			return
				reg == b->reg &&
				index == b->index &&
				scale == b->scale;
		}

		return (m_type == type::GPR || m_type == type::XMM) ? reg == b->reg : false;
	}

	instruction_operand::type::type() : m_type(NONE) {}
	instruction_operand::type::type(underlying type) : m_type(type) {}

	auto instruction_operand::type::to_string() const->std::string {
		switch (m_type) {
			case NONE:   return "NONE";
			case FLAGS:  return "FLAGS";
			case GPR:    return "GPR";
			case XMM:    return "XMM";
			case IMM:    return "IMM";
			case MEM:    return "MEM";
			case GLOBAL: return "GLOBAL";
			case ABS:    return "ABS";
			case LABEL:  return "LABEL";
			default: NOT_IMPLEMENTED();
		}

		return "";
	}

	instruction_operand::type::operator instruction_operand::type::underlying() const {
		return m_type;
	}

  void instruction_operand::set_type(type type) {
		m_type = type;
  }

	auto instruction_operand::get_type() const -> type {
		return m_type;
	}

	auto instruction_operand::create_label(const codegen_context& context, u64 target) -> handle<instruction_operand> {
		const handle<instruction_operand> operand = context.create_instruction_operand<label>();
		operand->set_type(type::LABEL);
		operand->get<label>().value = target;
		return operand;
	}

	auto instruction_operand::create_imm(const codegen_context& context, i32 imm) -> handle<instruction_operand> {
		const handle<instruction_operand> operand = context.create_instruction_operand();
		operand->set_type(type::IMM);
		operand->immediate = imm;
		return operand;
	}

	auto instruction_operand::create_abs(const codegen_context& context, u64 abs) -> handle<instruction_operand> {
		const handle<instruction_operand> operand = context.create_instruction_operand<absolute>();
		operand->set_type(type::ABS);
		operand->get<absolute>().value = abs;
		return operand;
	}

	bool operator==(handle<instruction_operand> operand, instruction_operand::type::underlying type) {
		return operand->get_type() == type;
	}
} // namespace sigma::ir
