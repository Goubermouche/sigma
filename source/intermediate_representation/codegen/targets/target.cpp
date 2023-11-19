#include "target.h"

// x64
#include "intermediate_representation/codegen/architectures/x64/x64_disassembler.h"
#include "intermediate_representation/codegen/architectures/x64/x64.h"

namespace ir {
	target target::create_target(arch arch, system system) {
		SUPPRESS_C4100(system);
		target target;
		
		target.m_architecture = pick_architecture(arch);
		target.m_disassembler = pick_disassembler(arch);

		return target;
	}

	auto target::emit_bytecode(codegen_context& context) const -> utility::byte_buffer {
		return m_architecture->emit_bytecode(context);
	}

	auto target::get_register_intervals() const -> std::vector<live_interval> {
		return m_architecture->get_register_intervals();
	}

	void target::select_instructions(codegen_context& context) const {
		return m_architecture->select_instructions(context);
	}

	auto target::disassemble(const utility::byte_buffer& bytecode, const codegen_context& context) const -> utility::string {
		return m_disassembler->disassemble(bytecode, context);
	}

	auto target::pick_architecture(arch arch) -> s_ptr<architecture> {
		switch (arch) {
			case arch::x64: return std::make_shared<x64_architecture>();
		}

		ASSERT(false, "specified architecture is not implemented");
		return nullptr;
	}

	auto target::pick_disassembler(arch arch) -> s_ptr<disassembler> {
		switch (arch) {
			case arch::x64: return std::make_shared<x64_disassembler>();
		}

		ASSERT(false, "specified disassembler is not implemented");
		return nullptr;
	}
}
