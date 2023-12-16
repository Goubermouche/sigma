#include "codegen_target.h"
#include "intermediate_representation/module.h"

// x64
#include "intermediate_representation/target/arch/x64/x64_disassembler.h"
#include "intermediate_representation/target/arch/x64/x64.h"

// object files
#include "intermediate_representation/codegen/outputs/coff.h"
#include "intermediate_representation/codegen/outputs/elf.h"

namespace sigma::ir {
	codegen_target::codegen_target(target target) : m_target(target) {
		m_object_file_emitter = pick_object_file_emitter(m_target.get_system());
		m_architecture        = pick_architecture(m_target.get_arch());
		m_disassembler        = pick_disassembler(m_target.get_arch());
	}

	auto codegen_target::get_target() const -> target {
		return m_target;
	}

	auto codegen_target::emit_bytecode(codegen_context& context) const -> utility::byte_buffer {
		ASSERT(m_architecture != nullptr, "target is not initialized");
		return m_architecture->emit_bytecode(context);
	}

	auto codegen_target::get_register_intervals() const -> std::vector<live_interval> {
		ASSERT(m_architecture != nullptr, "target is not initialized");
		return m_architecture->get_register_intervals();
	}

	void codegen_target::select_instructions(codegen_context& context) const {
		ASSERT(m_architecture != nullptr, "target is not initialized");
		return m_architecture->select_instructions(context);
	}

	auto codegen_target::disassemble(const utility::byte_buffer& bytecode, const codegen_context& context) const -> utility::string {
		ASSERT(m_disassembler != nullptr, "target is not initialized");
		return m_disassembler->disassemble(bytecode, context);
	}

	auto codegen_target::emit_object_file(module& module) const -> utility::object_file {
		ASSERT(m_object_file_emitter != nullptr, "target is not initialized");
		return m_object_file_emitter->emit(module);
	}

	auto codegen_target::pick_object_file_emitter(system system) -> s_ptr<object_file_emitter> {
		switch (system) {
			case system::WINDOWS: return std::make_shared<coff_file_emitter>();
			case system::LINUX:   return std::make_shared<elf_file_emitter>();
		}

		NOT_IMPLEMENTED();
		return nullptr;
	}

	auto codegen_target::pick_architecture(arch arch) -> s_ptr<architecture> {
		switch (arch) {
			case arch::X64: return std::make_shared<x64_architecture>();
		}

		NOT_IMPLEMENTED();
		return nullptr;
	}

	auto codegen_target::pick_disassembler(arch arch) -> s_ptr<disassembler> {
		switch (arch) {
			case arch::X64: return std::make_shared<x64_disassembler>();
		}

		NOT_IMPLEMENTED();
		return nullptr;
	}
} // namespace sigma::ir
