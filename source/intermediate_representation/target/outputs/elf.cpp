#include "elf.h"
#include "intermediate_representation/module.h"

#define WRITE(data, size) (memcpy(&output[write_position], data, size), write_position += (size))

namespace sigma::ir {
	utility::byte_buffer elf_file_emitter::emit(module& module) {
		std::vector<handle<external>> externals = module.generate_externals();

		// determine the machine type
		u16 machine = 0;

		switch (module.get_target().get_arch()) {
			case arch::X64: machine = EM_X86_64; break;
		}

		utility::byte_buffer string_table;
		string_table.push_back(0); // null string

		elf64_e_header header = {
			.type = ET_REL, // relocatable
			.machine = machine,
			.version = 1,
			.entry = 0,
			.shoff = 0,
			.flags = 0,
			.ehsize = sizeof(elf64_e_header),
			.shentsize = sizeof(elf64_s_header),
			.shstrndx = 1,
		};

		// setup .ident
		header.ident[EI_MAG0] = 0x7F;
		header.ident[EI_MAG1] = 'E';
		header.ident[EI_MAG2] = 'L';
		header.ident[EI_MAG3] = 'F';
		header.ident[EI_CLASS] = 2;
		header.ident[EI_DATA] = 1;
		header.ident[EI_VERSION] = 1;
		header.ident[EI_OSABI] = 0;
		header.ident[EI_ABIVERSION] = 0;

		// accumulate all sections
		module_output sections = module.get_output();
		u64 section_count = sections.sections.size() + 2;
		u64 output_size = sizeof(elf64_e_header);

		for(u64 i = 0; i < sections.sections.size(); ++i) {
			sections.sections[i].section_index = static_cast<u16>(3 + i);
			sections.sections[i].raw_data_pos = static_cast<u32>(output_size);
			output_size += sections.sections[i].total_size;
		}

		// calculate the relocation layout
		// each section with relocations needs a matching .rel section
		output_size = layout_relocations(
			sections.sections,
			static_cast<u32>(output_size), 
			sizeof(elf64_relocation)
		);

		for (module_section& section : sections.sections) {
			if (section.relocation_count > 0) {
				section_count += 1;
				string_table.append_string(".rela");
			}

			section.name_position = static_cast<u32>(string_table.get_size());
			string_table.append_string_nt(section.name);
		}

		// calculate symbol IDs
		utility::byte_buffer local_symbol_table = 
			utility::byte_buffer::zero_initialize(sizeof(elf64_symbol));

		utility::byte_buffer global_symbol_table;

		for (u64 i = 0; i < sections.sections.size(); ++i) {
			put_symbol(local_symbol_table, sections.sections[i].name_position, ELF64_ST_INFO(ELF64_STB_LOCAL, ELF64_STT_SECTION), static_cast<u16>(1 + i), 0, 0);
		}

		// .rela sections
		for (u64 i = 0; i < sections.sections.size(); ++i) {
			if(sections.sections[i].relocation_count) {
				put_symbol(local_symbol_table, sections.sections[i].name_position - 5, ELF64_ST_INFO(ELF64_STB_LOCAL, ELF64_STT_SECTION), static_cast<u16>(1 + i), 0, 0);
			}
		}

		put_section_symbols(sections.sections, string_table, local_symbol_table, ELF64_STB_LOCAL);
		put_section_symbols(sections.sections, string_table, global_symbol_table, ELF64_STB_GLOBAL);

		for(const handle<external> ex : externals) {
			const u32 name = static_cast<u32>(string_table.get_size());
			string_table.append_string_nt(ex->symbol.name);
			ex->symbol.id = global_symbol_table.get_size() / sizeof(elf64_symbol);

			put_symbol(global_symbol_table, name, ELF64_ST_INFO(ELF64_STB_GLOBAL, 0), 0, 0, 0);
		}

		u32 symbol_table_name = static_cast<u32>(string_table.get_size());
		string_table.append_string_nt(".symtab");

		// initialize the strtab
		elf64_s_header strtab;

		strtab.name = static_cast<u32>(string_table.get_size());
		string_table.append_string_nt(".strtab");

		strtab.type = SHT_STRTAB;
		strtab.flags = 0;
		strtab.addralign = 1;
		strtab.size = string_table.get_size();
		strtab.offset = output_size;

		output_size += string_table.get_size();

		// initialize the symtab
		elf64_s_header symtab;

		symtab.name = symbol_table_name;
		symtab.type = SHT_SYMTAB;
		symtab.flags = 0;
		symtab.addralign = 1;
		symtab.link = 1;
		symtab.info = static_cast<u32>(local_symbol_table.get_size() / sizeof(elf64_symbol));
		symtab.size = local_symbol_table.get_size() + global_symbol_table.get_size();
		symtab.entsize = sizeof(elf64_symbol);
		symtab.offset = output_size;

		output_size += local_symbol_table.get_size();
		output_size += global_symbol_table.get_size();

		header.shoff = output_size;
		header.shnum = static_cast<u16>(section_count + 1);

		// sections plus the NULL section at the start
		output_size += (1 + section_count) * sizeof(elf64_s_header);

		// write the output
		u64 write_position = 0;
		utility::byte_buffer output_buffer = utility::byte_buffer::zero_initialize(output_size);
		utility::byte* output = output_buffer.get_data();

		WRITE(&header, sizeof(header));

		// write section contents
		for (module_section& section : sections.sections) {
			write_position = helper_write_section(
				write_position, 
				&section,
				section.raw_data_pos,
				output_buffer
			);
		}

		// write relocation arrays
		u64 local_symbol_count = local_symbol_table.get_size() / sizeof(elf64_symbol);
		for (const module_section& section : sections.sections) {
			if(section.relocation_count > 0) {
				ASSERT(section.relocation_position == write_position, "invalid relocations");
				auto relocations = reinterpret_cast<elf64_relocation*>(&output[write_position]);

				for(const handle<compiled_function>& function : section.functions) {
					u64 source_offset = function->code_position;

					for(handle<symbol_patch> patch = function->first_patch; patch; patch = patch->next) {
						if(patch->internal) {
							continue;
						}

						u64 actual_position = source_offset + patch->pos;
						u64 symbol_id = patch->target->id;

						if(patch->target->is_non_local()) {
							symbol_id += local_symbol_count;
						}

						ASSERT(symbol_id != 0, "invalid symbol id");

						elf_relocation_type type;
						if(patch->target->type == symbol::GLOBAL) {
							type = ELF_X86_64_PC32;
						}
						else {
							type = ELF_X86_64_PLT32;
						}

						*relocations++ = elf64_relocation{
							.offset = actual_position,
							.info = ELF64_R_INFO(symbol_id, type),
							.addend = -4
						};
					}
				}

				write_position += section.relocation_count * sizeof(elf64_relocation);
			}
		}

		ASSERT(write_position == strtab.offset, "invalid write position");
		WRITE(string_table.get_data(), string_table.get_size());

		ASSERT(write_position == symtab.offset, "invalid write position");
		WRITE(local_symbol_table.get_data(), local_symbol_table.get_size());
		WRITE(global_symbol_table.get_data(), global_symbol_table.get_size());

		// write the section header
		std::memset(&output[write_position], 0, sizeof(elf64_s_header));
		write_position += sizeof(elf64_s_header);

		WRITE(&strtab, sizeof(strtab));
		WRITE(&symtab, sizeof(symtab));

		for (const module_section& section : sections.sections) {
			elf64_s_header sec = {
				.name = section.name_position,
				.type = SHT_PROGBITS,
				.flags = SHF_ALLOC,
				.offset = section.raw_data_pos,
				.size = section.total_size,
				.addralign = 16,
			};

			if (section.flags & module_section::WRITE) {
				sec.flags |= SHF_WRITE;
			}

			if (section.flags & module_section::EXEC) {
				sec.flags |= SHF_EXECINSTR;
			}

			WRITE(&sec, sizeof(sec));
		}

		for (u64 i = 0; i < sections.sections.size(); ++i) {
			if (sections.sections[i].relocation_count > 0) {
				elf64_s_header sec = {
					 .name = sections.sections[i].name_position - 5,
					 .type = SHT_RELA,
					 .flags = SHF_INFO_LINK,
					 .offset = sections.sections[i].relocation_position,
					 .size = sections.sections[i].relocation_count * sizeof(elf64_relocation),
					 .link = 2,
					 .info = static_cast<u32>(3 + i),
					 .addralign = 16,
					 .entsize = sizeof(elf64_relocation)
				};

				WRITE(&sec, sizeof(sec));
			}
		}

		ASSERT(write_position == output_size, "invalid write position");
		return { output_buffer };
	}

	u64 elf_file_emitter::put_symbol(utility::byte_buffer& stab, u32 name, u8 sym_info, u16 section_index, u64 value, u64 size) {
		const elf64_symbol symbol = {
			 .name = name,
			 .info = sym_info,
			 .shndx = section_index,
			 .value = value,
			 .size = size
		};

		stab.append_type(symbol);
		return stab.get_size() / sizeof(elf64_symbol) - 1;
	}

	void elf_file_emitter::put_section_symbols(const std::vector<module_section>& sections, utility::byte_buffer& string_table, utility::byte_buffer& stab, i32 t) {
		for (const module_section& section : sections) {
			const u16 section_index = section.section_index;

			// insert function symbols
			for(const handle<compiled_function> function : section.functions) {
				const std::string name_str = function->parent->symbol.name;

				u32 name = 0;
				if(!name_str.empty()) {
					name = static_cast<u32>(string_table.get_size());
					string_table.append_string_nt(name_str);
				}

				function->parent->symbol.id = put_symbol(
					stab, 
					name,
					static_cast<u8>(ELF64_ST_INFO(t, ELF64_STT_FUNC)),
					section_index,
					function->code_position,
					function->bytecode.get_size()
				);
			}

			const linkage acceptable = t == ELF64_STB_GLOBAL ? linkage::PUBLIC : linkage::PRIVATE;

			// insert global symbols
			for(const handle<global> global : section.globals) {
				if(global->symbol.link != acceptable) {
					continue;
				}

				const u32 name = static_cast<u32>(string_table.get_size());
				string_table.append_string_nt(global->symbol.name);
			
				global->symbol.id = put_symbol(
					stab,
					name, 
					static_cast<u8>(ELF64_ST_INFO(t, ELF64_STT_OBJECT)), 
					section_index, 
					global->position,
					0
				);
			}
		}
	}
} // namespace sigma::ir
