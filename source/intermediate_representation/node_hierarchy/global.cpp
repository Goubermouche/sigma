#include "global.h"
#include "intermediate_representation/module.h"

namespace sigma::ir {
    global::global(ir::symbol sym) : symbol(sym) {}

    void global::set_storage(
		u8 section_handle, u32 storage_size, u32 storage_alignment, u32 max_objects
	) {
		position = 0;
		size = storage_size;
		alignment = storage_alignment;
		parent_section = section_handle;
		objects.reserve(max_objects);
	}

	auto global::add_region(u32 region_offset, u32 region_size) -> void* {
		void* ptr = utility::malloc(region_size);

		const init_object object {
			.type = init_object::REGION,
			.offset = region_offset,
			.region = {
				.size = region_size,
				.ptr = ptr
			}
		};

		objects.push_back(object);
		return ptr;
	}
} // namespace sigma::ir
