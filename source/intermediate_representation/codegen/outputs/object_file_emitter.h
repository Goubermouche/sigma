#pragma once
#include <utility/filesystem/file_types/object_file.h>
#include <utility/containers/byte_buffer.h>
#include <utility/containers/handle.h>

namespace sigma::ir {
	using namespace utility::types;

	struct module_section;
	struct compiled_function;
	class module;


	/**
	 * \brief Generic object file emitter, this base class should be specialized for individual
	 * targets (ELF, COFF, etc.)
	 */
	class object_file_emitter {
	public:
		virtual utility::object_file emit(module& module) = 0;
		virtual ~object_file_emitter() = default;
	protected:
		static auto layout_relocations(std::vector<module_section>& sections, u32 output_size, u32 relocation_size) -> u32;
		static auto helper_write_section(u64 write_pos, const module_section* section, u32 pos, utility::byte_buffer& buffer) -> u64;
		static auto emit_call_patches(handle<compiled_function> compiled_func) -> u32;
	};
} // namespace sigma::ir
