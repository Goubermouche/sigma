#pragma once
#include "intermediate_representation/codegen/codegen_context.h"

#include "utility/containers/byte_buffer.h"
#include "utility/containers/string.h"

namespace sigma::ir {
	class disassembler {
	public:
		virtual auto disassemble(
			const utility::byte_buffer& bytecode, const codegen_context& context
		) -> utility::string = 0;

		virtual ~disassembler() = default;
	};

	class architecture {
	public:
		virtual auto emit_bytecode(codegen_context& context) -> utility::byte_buffer = 0;
		virtual auto get_register_intervals() -> std::vector<live_interval> = 0;
		virtual void select_instructions(codegen_context& context) = 0;

		virtual ~architecture() = default;
	};
} // namespace sigma::ir
