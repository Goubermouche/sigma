#pragma once
#include "intermediate_representation/target/outputs/object_file_emitter.h"
#include "intermediate_representation/target/target.h"
#include <util/handle.h>

#define COFF_CHARACTERISTICS_TEXT   0x60500020u
#define COFF_CHARACTERISTICS_DATA   0xC0000040u
#define COFF_CHARACTERISTICS_RODATA 0x40000040u
#define COFF_CHARACTERISTICS_BSS    0xC0500080u
#define COFF_CHARACTERISTICS_DEBUG  0x40100040u

#define MD5_HASHBYTES 16

namespace sigma::ir {
	enum {
		UNWIND_FLAG_EHANDLER = 0x01,
		UNWIND_FLAG_UHANDLER = 0x02,
		UNWIND_FLAG_CHAININFO = 0x04,
	};

	struct coff {
		enum value {
			SECTION_NO_PAD = 0x00000008,
			SECTION_CODE = 0x00000020,
			SECTION_INIT = 0x00000040,
			SECTION_UNINIT = 0x00000080,
			SECTION_OTHER = 0x00000100,
			SECTION_INFO = 0x00000200,
			SECTION_REMOVE = 0x00000800,
			SECTION_COMDAT = 0x00001000,

			// this is actually a 4bit field
			SECTION_ALIGN = 0x00F00000,

			// if we have more than 65535 relocations we do this
			SECTION_RELOC_OVR = 0x00F00000,

			// memory flags
			SECTION_DISCARDABLE = 0x02000000,
			SECTION_NOT_CACHED = 0x04000000,
			SECTION_NOT_PAGED = 0x08000000,
			SECTION_SHARED = 0x10000000,
			SECTION_EXECUTE = 0x20000000,
			SECTION_READ = 0x40000000,
			SECTION_WRITE = 0x80000000,
		};
	};

	struct unwind_op {
		enum value {
			PUSH_NONVOL = 0, // info == register number
			ALLOC_LARGE,     // no info, alloc size in next 2 slots
			ALLOC_SMALL,     // info == size of allocation / 8 - 1
			SET_FPREG,       // no info, FP = RSP + UNWIND_INFO.FPRegOffset*16
			SAVE_NONVOL,     // info == register number, offset in next slot
			SAVE_NONVOL_FAR, // info == register number, offset in next 2 slots
			SAVE_XMM128 = 8, // info == XMM reg number, offset in next slot
			SAVE_XMM128_FAR, // info == XMM reg number, offset in next 2 slots
			PUSH_MACHFRAME   // info == 0: no error-code, 1: error-code
		};
	};

	struct coff_machine {
		enum value : u16 {
			NONE = 0,
			AMD64 = 0x8664, // AMD64 (K8)
			ARM64 = 0xAA64, // ARM64 Little-Endian
		};
	};

	#pragma pack(push, 2)
	struct coff_image_relocation {
		union {
			u32 virtual_address;
			u32 relocation_count;
		};

		u32 symbol_table_index;
		u16 type;
	};

	struct coff_symbol {
		union {
			u8 short_name[8];
			u32 long_name[2];
		};
		u32 value;
		i16 section_number;
		u16 type;
		u8 storage_class;
		u8 aux_symbols_count;
	};

	struct coff_section_header {
		char name[8];

		union {
			u32 physical_address;
			u32 virtual_size;
		} misc;

		u32 virtual_address;
		u32 raw_data_size;
		u32 raw_data_pos;
		u32 relocation_pointer;
		u32 line_number_pointer;
		u16 relocation_count;
		u16 line_number_count;
		u32 characteristics;
	};

	struct coff_file_header {
		u16 machine;
		u16 section_count;
		u32 timestamp;
		u32 symbol_table;
		u32 symbol_count;
		u16 optional_header_size;
		u16 flags;
	};
	#pragma pack(pop)

	struct string_table {
		u64 pos;
		u32 size;
	};

	struct coff_unwind_info {
		u64 section_num, patch_count;

		utility::byte_buffer xdata_chunk;
		utility::byte_buffer pdata_chunk;
		utility::byte_buffer pdata_relocations;

		coff_section_header xdata_header;
		coff_section_header pdata_header;
	};

	struct offset {
		u8 code_offset;
		u8 unwind_op : 4;
		u8 op_info : 4;
	};

	typedef union {
		offset o; // offset
		u16 frame_offset;
	} unwind_code;

	struct unwind_info {
		u8 version : 3;
		u8 flags : 5;
		u8 prolog_length;
		u8 code_count;
		u8 frame_register : 4;
		u8 frame_offset : 4;
	};

#pragma pack(push, 2)
	struct coff_auxiliary_section_symbol {
		u32 length;            // section length
		u16 relocation_count;  // number of relocation entries
		u16 line_number_count; // number of line numbers
		u32 checksum;          // checksum for communal
		i16 number;            // section number to associate with
		u8 selection;          // communal selection type
		u8 reserved;
		u16 high_bits;         // high bits of the section number
	};
#pragma pack(pop)

	class coff_file_emitter : public object_file_emitter {
	public:
		utility::byte_buffer emit(module& module) override;
	private:
		static auto generate_unwind_info(module& module, u64 xdata_section, const module_section& section) -> handle<coff_unwind_info>;
		static void emit_win_unwind_info(utility::byte_buffer& buffer, handle<compiled_function> function, u64 stack_usage);
		static auto machine_to_coff_machine(target target) -> coff_machine::value;
	};
} // namespace sigma::ir
