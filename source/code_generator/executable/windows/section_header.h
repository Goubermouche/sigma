#pragma once
#include <utility/macros.h>
#include <windows.h>

using namespace utility::types;

namespace sigma::windows {

	enum class section_header_characteristics : u32 {
		type_no_pad            = 0x00000008,

		cnt_code               = 0x00000020,
		cnt_initialized_data   = 0x00000040,
		cnt_uninitialized_data = 0x00000080,

		lnk_other              = 0x00000100,
		lnk_info               = 0x00000200,
		lnk_remove             = 0x00000800,
		lnk_comdat             = 0x00001000,

		no_defer_spec_exc      = 0x00004000,
		gprel                  = 0x00008000,
		mem_fardata            = 0x00008000,
		mem_purgeable          = 0x00020000,
		mem_16_bit             = 0x00020000,
		mem_locked             = 0x00040000,
		mem_preload            = 0x00080000,

		align_1_byte           = 0x00100000,
		align_2_bytes          = 0x00200000,
		align_4_bytes          = 0x00300000,
		align_8_bytes          = 0x00400000,
		align_16_bytes         = 0x00500000, // default alignment
		align_32_bytes         = 0x00600000,
		align_64_bytes         = 0x00700000,
		align_128_bytes        = 0x00800000,
		align_256_bytes        = 0x00900000,
		align_512_bytes        = 0x00A00000,
		align_1024_bytes       = 0x00B00000,
		align_2048_bytes       = 0x00C00000,
		align_4096_bytes       = 0x00D00000,
		align_8192_bytes       = 0x00E00000,

		align_mask             = 0x00F00000,
		lnk_nreloc_ovfl        = 0x01000000,
		mem_discardable        = 0x02000000,
		mem_not_cached         = 0x04000000,
		mem_not_paged          = 0x08000000,
		mem_shared             = 0x10000000,
		mem_execute            = 0x20000000,
		mem_read               = 0x40000000,
		mem_write              = 0x80000000
	};

	section_header_characteristics operator|(
		section_header_characteristics lhs,
		section_header_characteristics rhs
	);

	section_header_characteristics operator&(
		section_header_characteristics lhs,
		section_header_characteristics rhs
	);

	bool operator!=(
		section_header_characteristics lhs, 
		u32 rhs
	);

	bool operator==(
		section_header_characteristics lhs, 
		u32 rhs
	);

	struct section_header {
		void print() const;

		u8 name[8];
		u32 virtual_size;
		u32 virtual_address;
		u32 raw_data_size;
		u32 raw_data_pointer;
		u32 relocation_pointer;
		u32 line_number_pointer;
		u16 relocation_count;
		u16 line_number_count;
		section_header_characteristics characteristics;
	};
}