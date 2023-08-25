#pragma once
#include <utility/diagnostics/console.h>

using namespace utility::types;

namespace sigma::windows {
	enum class file_header_characteristics : u16 {
		relocs_stripped         = 0x0001,
		executable              = 0x0002,
		line_nums_stripped      = 0x0004,
		local_syms_stripped     = 0x0008,
		aggressive_ws_trim      = 0x0010,
		large_address_aware     = 0x0020,
		bytes_reversed_lo       = 0x0080,
		machine_32_bit          = 0x0100,
		debug_stripped          = 0x0200,
		removable_run_from_swap = 0x0400,
		net_run_from_swap       = 0x0800,
		system                  = 0x1000,
		dll                     = 0x2000,
		up_system_only          = 0x4000,
		bytes_reversed_hi       = 0x8000
	};

	file_header_characteristics operator|(
		file_header_characteristics lhs, 
		file_header_characteristics rhs
	);

	file_header_characteristics operator&(
		file_header_characteristics lhs, 
		file_header_characteristics rhs
	);

	enum class file_header_machine : u16 {
		unknown    = 0,
		targethost = 0x0001,
		i386       = 0x014c,
		r3000      = 0x0162,
		r4000      = 0x0166,
		r10000     = 0x0168,
		wcemipsv2  = 0x0169,
		alpha      = 0x0184,
		sh3        = 0x01a2,
		sh3_dsp    = 0x01a3,
		ds3_e      = 0x01a4,
		sh4        = 0x01a6,
		sh5        = 0x01a8,
		arm        = 0x01c0,
		thumb      = 0x01c2,
		armnt      = 0x01c4,
		am33       = 0x01d3,
		powerpc    = 0x01F0,
		powerpcfp  = 0x01f1,
		ia64       = 0x0200,
		mips16     = 0x0266,
		alpha64    = 0x0284,
		mipsfpu    = 0x0366,
		mipsfpu16  = 0x0466,
		axp_64     = 0x0284,
		tricore    = 0x0520,
		cef        = 0x0CEF,
		ebc        = 0x0EBC,
		amd64      = 0x8664,
		m32r       = 0x9041,
		arm64      = 0xAA64,
		cee        = 0xC0EE
	};

	file_header_machine get_machine_type();

	struct file_header {
		void print() const;

		file_header_machine machine;
		u16 section_count;
		u32 time_date_stamp;
		u32 pointer_to_symbol_table;
		u32 symbol_count;
		u16 optional_header_size;
		file_header_characteristics characteristics;
	};
}
