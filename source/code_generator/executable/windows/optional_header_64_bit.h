#pragma once
#include <utility/diagnostics/console.h>

using namespace utility::types;

namespace sigma::windows {
	struct data_directory {
		u32 virtual_address;
		u32 size;
	};

	struct optional_header_64_bit {
		void print() const;

		// standard fields
		u16 magic;
		u8 major_linker_version;
		u8 minor_linker_version;
		u32 code_size;
		u32 initialized_data_size;
		u32 uninitialized_data_size;
		u32 entry_point_address;
		u32 code_base;

		// additional NT fields
		u64 image_base;
		u32 section_alignment;
		u32 file_alignment;
		u16 major_os_version;
		u16 minor_os_version;
		u16 major_image_version;
		u16 minor_image_version;
		u16 major_subsystem_version;
		u16 minor_subsystem_version;
		u32 win32_version_value;
		u32 image_size;
		u32 header_size;
		u32 check_sum;
		u16 subsystem;
		u16 DLL_characteristics;
		u64 stack_reserve_size;
		u64 stack_commit_size;
		u64 heap_reserve_size;
		u64 heap_commit_size;
		u32 loader_flags;                    // must be zero
		u32 RVA_and_size_count;              // must be 0x10 for PE32/PE32+
		data_directory data_directories[16]; // array of data directories
	};
}
