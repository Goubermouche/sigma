#pragma once
#include "intermediate_representation/codegen/outputs/object_file_emitter.h"

namespace sigma::ir {
#define EM_NONE    0   // unknown machine
#define EM_X86_64  62  // advanced Micro Devices x86-64
#define EM_AARCH64 183 // AArch64 (64-bit ARM)

#define EI_MAG0       0
#define EI_MAG1       1
#define EI_MAG2       2
#define EI_MAG3       3
#define EI_CLASS      4  // class of machine
#define EI_DATA       5  // data format
#define EI_VERSION    6  // ELF format version
#define EI_OSABI      7  // operating system / ABI identification
#define EI_ABIVERSION 8  // ABI version
#define OLD_EI_BRAND  8  // start of architecture identification
#define EI_PAD        9  // start of padding (per SVR4 ABI)
#define EI_NIDENT     16 // size of e_ident array

#define ET_NONE   0      // unknown type
#define ET_REL    1      // relocatable
#define ET_EXEC   2      // executable
#define ET_DYN    3      // shared object
#define ET_CORE   4      // core file
#define ET_LOOS   0xfe00 // first operating system specific
#define ET_HIOS   0xfeff // last operating system-specific
#define ET_LOPROC 0xff00 // first processor-specific
#define ET_HIPROC 0xffff // last processor-specific

#define SHF_WRITE            0x1        // section contains writable data
#define SHF_ALLOC            0x2        // section occupies memory
#define SHF_EXECINSTR        0x4        // section contains instructions
#define SHF_MERGE            0x10       // section may be merged
#define SHF_STRINGS          0x20       // section contains strings
#define SHF_INFO_LINK        0x40       // sh_info holds section index
#define SHF_LINK_ORDER       0x80       // special ordering requirements
#define SHF_OS_NONCONFORMING 0x100      // OS-specific processing required
#define SHF_GROUP            0x200      // member of section group
#define SHF_TLS              0x400      // section contains TLS data
#define SHF_MASKOS           0x0ff00000 // OS-specific semantics
#define SHF_MASKPROC         0xf0000000 // processor-specific semantics

// ST_TYPE
#define ELF64_STT_NOTYPE  0
#define ELF64_STT_OBJECT  1
#define ELF64_STT_FUNC    2
#define ELF64_STT_SECTION 3

// SH_TYPE
#define SHT_NULL     0 // inactive
#define SHT_PROGBITS 1 // program defined information
#define SHT_SYMTAB   2 // symbol table section
#define SHT_STRTAB   3 // string table section
#define SHT_RELA     4 // relocation section with addends
#define SHT_NOBITS   8 // no space section

// ST_INFO
#define ELF64_STB_LOCAL  0
#define ELF64_STB_GLOBAL 1
#define ELF64_STB_WEAK   2

#define ELF64_ST_INFO(b, t) (((b) << 4) | ((t) & 0xF))
#define ELF64_R_INFO(s, t) (((u64)(s) << 32ULL) + ((u64)(t) & 0xffffffffULL))

	struct elf64_e_header {
		u8 ident[16];
		u16 type;
		u16 machine;
		u32 version;
		u64 entry;
		u64 phoff;
		u64 shoff;
		u32 flags;
		u16 ehsize;
		u16 phentsize;
		u16 phnum;
		u16 shentsize;
		u16 shnum;
		u16 shstrndx;
	};

	struct elf64_s_header {
		u32 name;
		u32 type;
		u64 flags;
		u64 addr;
		u64 offset;
		u64 size;
		u32 link;
		u32 info;
		u64 addralign;
		u64 entsize;
	};

	struct elf64_relocation{
		u64 offset;
		u64 info;
		i64 addend;
	};

	struct elf64_symbol {
		u32 name;
		u8 info;
		u8 other;
		u16 shndx;
		u64 value;
		u64 size;
	};

	enum elf_relocation_type {
		ELF_X86_64_NONE = 0,
		ELF_X86_64_64 = 1,
		ELF_X86_64_PC32 = 2,
		ELF_X86_64_GOT32 = 3,
		ELF_X86_64_PLT32 = 4,
		ELF_X86_64_GOTPCREL = 9,
	};

	class elf_file_emitter : public object_file_emitter {
	public:
		utility::object_file emit(module& module) override;
	private:
		static u64 put_symbol(utility::byte_buffer& stab, u32 name, u8 sym_info, u16 section_index, u64 value, u64 size);
		static void put_section_symbols(const std::vector<module_section>& sections, utility::byte_buffer& string_table,utility::byte_buffer& stab, i32 t);
	};
} // namespace sigma::ir

