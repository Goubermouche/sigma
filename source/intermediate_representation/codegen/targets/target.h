#pragma once
#include "intermediate_representation/codegen/architectures/architecture.h"

namespace ir {
	enum class arch {
		x64 // x86/x64
	};

	enum class system {
		windows,
		linux,
		mac_os,
		android,
		web
	};

	class target {
	public:
		static target create_target(arch arch, system system);

		auto emit_bytecode(codegen_context& context) const-> utility::byte_buffer;
		auto get_register_intervals() const-> std::vector<live_interval>;
		void select_instructions(codegen_context& context) const;

		auto disassemble(const utility::byte_buffer& bytecode, const codegen_context& context) const -> utility::string;
	private:
		static auto pick_architecture(arch arch) -> s_ptr<architecture>;
		static auto pick_disassembler(arch arch) -> s_ptr<disassembler>;
	private:
		s_ptr<architecture> m_architecture;
		s_ptr<disassembler> m_disassembler;
	};
}
