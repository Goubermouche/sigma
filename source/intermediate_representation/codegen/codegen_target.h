#pragma once
#include "intermediate_representation/codegen/object_file_emitter.h"
#include "intermediate_representation/target/arch/architecture.h"
#include "intermediate_representation/target/target.h"

namespace sigma::ir {
	class module;
	class codegen_target {
	public:
		codegen_target() = default;
		codegen_target(target target);

		[[nodiscard]] auto get_target() const -> target;

		auto emit_bytecode(codegen_context& context) const-> utility::byte_buffer;
		auto get_register_intervals() const-> std::vector<live_interval>;
		void select_instructions(codegen_context& context) const;

		auto disassemble(const utility::byte_buffer& bytecode, const codegen_context& context) const -> utility::string;

		auto emit_object_file(module& module) const -> utility::object_file;
	private:
		static auto pick_object_file_emitter(system system) -> s_ptr<object_file_emitter>;
		static auto pick_architecture(arch arch) -> s_ptr<architecture>;
		static auto pick_disassembler(arch arch) -> s_ptr<disassembler>;
	private:
		s_ptr<object_file_emitter> m_object_file_emitter;
		s_ptr<architecture> m_architecture;
		s_ptr<disassembler> m_disassembler;

		target m_target;
	};
}
