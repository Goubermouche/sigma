#include "global.h"
#include "intermediate_representation/module.h"

namespace sigma::ir {
	void global::set_storage(
		u8 section_handle, u32 storage_size, u32 storage_alignment, u32 max_objects
	) {
		// parent = section
		position = 0;
		size = storage_size;
		alignment = storage_alignment;
		parent_section = section_handle;
		objects.reserve(max_objects);
	}

	auto global::add_region(u32 region_offset, u32 region_size) -> void* {
		void* ptr = malloc(region_size);

		init_object o {
			.type = init_object::REGION,
			.offset = region_offset,
			.r = {
				.size = region_size,
				.ptr = ptr
			}
		};

		objects.push_back(o);
		return ptr;
	}
} // namespace sigma::ir
