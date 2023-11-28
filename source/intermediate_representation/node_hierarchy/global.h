#pragma once
#include "intermediate_representation/node_hierarchy/symbol.h"
#include "types.h"

namespace ir {
	struct init_object {
		enum init_object_type {
			REGION,
			RELOCATION
		};

		handle<symbol> relocation;
		init_object_type type;
		u32 offset;

		struct {
			u32 size;
			const void* ptr;
		} r;
	};

	enum linkage : u8 {
		PUBLIC,
		PRIVATE,
		SO_LOCAL, // exports to the rest of the shared object
		SO_EXPORT // exports outside of the shared object
	};

	class module;
	struct module_section;

	struct global {
		void set_storage(u8 section_handle, u32 storage_size, u32 storage_alignment, u32 max_objects);
		auto add_region(u32 region_offset, u32 region_size) -> void*;

		symbol symbol;
		linkage linkage;

		u8 parent_section;

		u32 position;
		u32 size;
		u32 alignment;

		// contents 
		std::vector<init_object> objects;
	};

	struct external {
		symbol symbol;
		linkage linkage;
	};
}
