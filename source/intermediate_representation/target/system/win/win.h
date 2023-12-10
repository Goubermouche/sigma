#pragma once
#include "intermediate_representation/target/parameter_descriptor.h"
#include "intermediate_representation/target/arch/x64/x64.h"

#define IMAGE_SCN_LNK_NRELOC_OVFL 0x01000000

#define IMAGE_SYM_CLASS_EXTERNAL      0x0002
#define IMAGE_SYM_CLASS_STATIC        0x0003
#define IMAGE_SYM_CLASS_LABEL         0x0006
#define IMAGE_SYM_CLASS_FILE          0x0067
#define IMAGE_SYM_CLASS_SECTION       0x0068
#define IMAGE_SYM_CLASS_WEAK_EXTERNAL 0x0069

#define IMAGE_FILE_LINE_NUMS_STRIPPED 0x0004

#define IMAGE_REL_AMD64_ADDR64   0x0001
#define IMAGE_REL_AMD64_ADDR32   0x0002
#define IMAGE_REL_AMD64_ADDR32NB 0x0003
#define IMAGE_REL_AMD64_REL32    0x0004
#define IMAGE_REL_AMD64_REL32_1  0x0005
#define IMAGE_REL_AMD64_REL32_2  0x0006
#define IMAGE_REL_AMD64_REL32_3  0x0007
#define IMAGE_REL_AMD64_REL32_4  0x0008
#define IMAGE_REL_AMD64_REL32_5  0x0009
#define IMAGE_REL_AMD64_SECTION  0x000A
#define IMAGE_REL_AMD64_SECREL   0x000B

#define IMAGE_SCN_LNK_REMOVE      0x00000800
#define IMAGE_SCN_LNK_COMDAT      0x00001000
#define IMAGE_SCN_MEM_DISCARDABLE 0x02000000
#define IMAGE_SCN_MEM_EXECUTE     0x20000000
#define IMAGE_SCN_MEM_READ        0x40000000
#define IMAGE_SCN_MEM_WRITE       0x80000000

#define IMAGE_SCN_CNT_CODE               0x00000020 // section contains code
#define IMAGE_SCN_CNT_INITIALIZED_DATA   0x00000040 // section contains initialized data
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA 0x00000080 // section contains uninitialized data

#define IMAGE_DIRECTORY_ENTRY_EXPORT         0  // export directory
#define IMAGE_DIRECTORY_ENTRY_IMPORT         1  // import directory
#define IMAGE_DIRECTORY_ENTRY_RESOURCE       2  // resource directory
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION      3  // exception directory
#define IMAGE_DIRECTORY_ENTRY_SECURITY       4  // security directory
#define IMAGE_DIRECTORY_ENTRY_BASERELOC      5  // base relocation table
#define IMAGE_DIRECTORY_ENTRY_DEBUG          6  // debug directory
#define IMAGE_DIRECTORY_ENTRY_ARCHITECTURE   7  // architecture specific data
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR      8  // RVA of GP
#define IMAGE_DIRECTORY_ENTRY_TLS            9  // TLS directory
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG    10 // load configuration directory
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT   11 // bound import directory in headers
#define IMAGE_DIRECTORY_ENTRY_IAT            12 // import address table
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT   13 // delay load import descriptors
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14 // COM runtime descriptor

#define IMAGE_SUBSYSTEM_WINDOWS_GUI 2
#define IMAGE_SUBSYSTEM_WINDOWS_CUI 3
#define IMAGE_SUBSYSTEM_EFI_APPLICATION 10

namespace sigma::ir::win {
	constexpr auto get_caller_saved() -> u16 {
		return
			1u << x64::RAX |
			1u << x64::RCX |
			1u << x64::RDX |
			1u << x64::R8  |
			1u << x64::R9  |
			1u << x64::R10 |
			1u << x64::R11;
	}

	static const parameter_descriptor parameter_descriptor = {
		.gpr_count = 4,
		.xmm_count = 4,
		.caller_saved_xmm_count = 6,
		.caller_saved_gpr_count = get_caller_saved(),
		.gpr_registers = {
			x64::RCX, x64::RDX, x64::R8, x64::R9, reg::invalid_id, reg::invalid_id
		}
	};
}
