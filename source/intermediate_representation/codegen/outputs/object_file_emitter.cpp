#include "object_file_emitter.h"
#include "intermediate_representation/module.h"

namespace sigma::ir {
	auto object_file_emitter::layout_relocations(std::vector<module_section>& sections, u32 output_size, u32 relocation_size) -> u32 {
		// calculate the relocation layout
		for (module_section& section : sections) {
			u32 relocation_count = 0;

			for (const handle<compiled_function> function : section.functions) {
				relocation_count += emit_call_patches(function);
			}

			for (const handle<global> global : section.globals) {
				for (const init_object& object : global->objects) {
					relocation_count += object.type == init_object::RELOCATION;
				}
			}

			section.relocation_count = relocation_count;
			section.relocation_position = output_size;

			output_size += relocation_count * relocation_size;
		}

		return output_size;
	}

	auto object_file_emitter::emit_call_patches(handle<compiled_function> compiled_func) -> u32 {
		const u64 source_section = compiled_func->parent->parent_section;
		u64 ordinal = 0;

		for (handle<symbol_patch> patch = compiled_func->first_patch; patch; patch = patch->next) {
			if (patch->target->type == symbol::symbol_type::FUNCTION) {
				const u64 destination_section = reinterpret_cast<function*>(patch->target.get())->output.parent->parent_section;

				// you can't do relocations across sections
				if (source_section == destination_section) {
					ASSERT(patch->pos < compiled_func->bytecode.get_size(), "invalid patch position");

					const u64 actual_position = compiled_func->code_position + patch->pos + 4;
					u32 position = static_cast<u32>(reinterpret_cast<function*>(patch->target.get())->output.code_position - actual_position);
					std::memcpy(&compiled_func->bytecode[patch->pos], &position, sizeof(uint32_t));

					ordinal += 1;
					patch->internal = true;
				}
			}
		}

		return static_cast<u32>(compiled_func->patch_count - ordinal);
	}

	auto object_file_emitter::helper_write_section(u64 write_pos, const module_section* section, u32 pos, utility::byte_buffer& buffer) -> u64 {
		ASSERT(write_pos == pos, "invalid write position");
		utility::byte* data = &buffer[pos];

		// place functions
		for (const handle<compiled_function> function : section->functions) {
			if (function != nullptr) {
				std::memcpy(data + function->code_position, function->bytecode.get_data(), function->bytecode.get_size());
			}
		}

		// place globals
		for (const handle<global> global : section->globals) {
			std::memset(&data[global->position], 0, global->size);

			for (const init_object& object : global->objects) {
				if (object.type == init_object::REGION) {
					ASSERT(object.offset + object.region.size <= global->size, "invalid object layout");
					memcpy(&data[global->position + object.offset], object.region.ptr, object.region.size);
				}
			}
		}

		return write_pos + section->total_size;
	}
} // namespace sigma::ir
