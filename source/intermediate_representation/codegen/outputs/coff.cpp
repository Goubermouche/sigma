#include "coff.h"
#include "intermediate_representation/target/system/win/win.h"
#include "intermediate_representation/module.h"

namespace ir {
	utility::object_file coff_file_emitter::emit(module& module) {
		std::vector<handle<external>> externals = module.generate_externals();

		auto sections = module.get_sections();
		u64 section_count = sections.size();

		std::vector<handle<coff_unwind_info>> unwinds(section_count);

		// mark correct flags on sections
		for (auto& section : sections) {
			u32 flags = coff::SECTION_READ;

			flags |= section.flags & module_section::WRITE ? coff::SECTION_WRITE : 0;
			flags |= section.flags & module_section::EXEC  ? coff::SECTION_EXECUTE | coff::SECTION_CODE : coff::SECTION_INIT;

			if (section.comdat.ty != comdat::NONE) {
				flags |= coff::SECTION_COMDAT;
			}

			section.export_flags = flags;
		}

		// generate unique ID's
		u64 unique_id_counter = section_count * 2;

		for (auto& section : sections) {
			if (!section.functions.empty()) {
				unique_id_counter += 4;
			}
		}

		for (auto& section : sections) {
			for (const auto& func : section.functions) {
				func->parent->symbol.symbol_id = unique_id_counter++;
			}

			for (const auto& global : section.globals) {
				global->symbol.symbol_id = unique_id_counter++;
			}
		}

		for (auto& ex : externals) {
			ex->symbol.symbol_id = unique_id_counter++;
		}

		for (u64 i = 0; i < sections.size(); ++i) {
			sections[i].section_index = static_cast<u8>(i + 1);

			// generate unwind information for each section with functions
			if (!sections[i].functions.empty()) {
				unwinds[i] = generate_unwind_info(module, section_count, sections[i]);
				section_count += 2; // .pdata + .xdata
			}
		}

		// COFF file header & section headers
		coff_file_header header = {
			.section_count = static_cast<u16>(section_count),
			.timestamp = 0,
			.symbol_table = 0,
			.symbol_count = static_cast<u32>(unique_id_counter),
			.flags = IMAGE_FILE_LINE_NUMS_STRIPPED
		};

		header.machine = machine_to_coff_machine(module.get_target());

		// calculate output size & layout object
		u32 output_size = sizeof(coff_file_header);
		output_size += static_cast<u32>(section_count) * sizeof(coff_section_header);

		for (u64 i = 0; i < sections.size(); ++i) {
			sections[i].raw_data_pos = output_size;
			output_size += sections[i].total_size;

			if (handle<coff_unwind_info> unwind = unwinds[i]) {
				unwind->pdata_header.raw_data_pos = output_size;
				output_size += unwind->pdata_header.raw_data_size;

				unwind->xdata_header.raw_data_pos = output_size;
				output_size += unwind->xdata_header.raw_data_size;

				unwind->pdata_header.relocation_pointer = output_size;
				output_size += static_cast<u32>(unwind->pdata_relocations.get_size());
			}
		}

		// calculate relocation layout
		output_size = layout_relocations(sections, output_size, sizeof(coff_image_relocation));

		header.symbol_table = output_size;
		output_size += header.symbol_count * sizeof(coff_symbol);

		// first 4 bytes of the table are the size of the table
		string_table string_table = {
			.pos = output_size,
			.size = 4
		};

		// compute string table size
		for (const auto& section : sections) {
			for (const auto& function : section.functions) {
				u64 name_length = function->parent->symbol.name.size();

				if (name_length >= 8) {
					string_table.size += static_cast<u32>(name_length) + 1;
				}
			}

			for (const auto& global : section.globals) {
				u64 name_length = global->symbol.name.size();

				if (name_length >= 8) {
					string_table.size += static_cast<u32>(name_length) + 1;
				}
			}
		}

		for (const auto& ex : externals) {
			u64 name_length = ex->symbol.name.size();

			if (name_length >= 8) {
				string_table.size += static_cast<u32>(name_length) + 1;
			}
		}

		output_size += string_table.size;

		// write the file
		utility::object_file out_file;
		utility::byte_buffer headers = utility::byte_buffer::zero_initialize(sizeof(coff_file_header) + sizeof(coff_section_header) * section_count);

		// write the file header 
		auto header_data = reinterpret_cast<coff_file_header*>(headers.get_data());
		*header_data = header;

		// write sections headers
		auto sec_headers = reinterpret_cast<coff_section_header*>(header_data + 1);

		for (auto& section : sections) {
			coff_section_header section_header = {
				.raw_data_size = section.total_size,
				.raw_data_pos = section.raw_data_pos,
				.relocation_pointer = section.relocation_position,
				.characteristics = section.export_flags
			};

			u64 section_name_length = section.name.size();
			memcpy(section_header.name, section.name.c_str(), section_name_length > 8 ? 8 : section_name_length);

			if (section.relocation_count >= 0xFFFF) {
				section_header.relocation_count = 0xFFFF;
				section_header.characteristics |= IMAGE_SCN_LNK_NRELOC_OVFL;
			}
			else {
				section_header.relocation_count = static_cast<u16>(section.relocation_count);
			}

			*sec_headers++ = section_header;
		}

		for (u64 i = 0; i < sections.size(); ++i) {
			if (unwinds[i]) {
				// .pdata
				memcpy(sec_headers + 0, &unwinds[i]->pdata_header, sizeof(coff_section_header));
				// .xdata
				memcpy(sec_headers + 1, &unwinds[i]->xdata_header, sizeof(coff_section_header));
				sec_headers += 2;
			}
		}

		out_file.append(headers);

		for (u64 i = 0; i < sections.size(); ++i) {
			utility::byte_buffer sec = utility::byte_buffer::zero_initialize(sections[i].total_size);
			helper_write_section(0, &sections[i], 0, sec);
			out_file.append(sec);

			if (handle<coff_unwind_info> unwind = unwinds[i]) {
				out_file.append(unwind->pdata_chunk);
				out_file.append(unwind->xdata_chunk);
				out_file.append(unwind->pdata_relocations);
			}
		}

		// write relocations
		for (auto& section : sections) {
			u32 relocation_count = section.relocation_count;
			utility::byte_buffer relocations(relocation_count * sizeof(coff_image_relocation));
			relocations.zero_fill();

			auto relocations_ptr = reinterpret_cast<coff_image_relocation*>(relocations.get_data());

			for (const auto& function : section.functions) {
				u64 source_offset = function->code_position;

				for (handle<symbol_patch> patch : function->patches) {
					if (patch->internal) {
						continue;
					}

					u64 actual_pos = source_offset + patch->pos;
					u64 symbol_id = patch->target->symbol_id;
					ASSERT(symbol_id != 0, "invalid symol ID");

					if (patch->target->tag == symbol::symbol_tag::FUNCTION || patch->target->tag == symbol::symbol_tag::EXTERNAL) {
						*relocations_ptr++ = coff_image_relocation{
							.virtual_address = static_cast<u32>(actual_pos),
							.symbol_table_index = static_cast<u32>(symbol_id),
							.type = IMAGE_REL_AMD64_REL32,
						};
					}
					else if (patch->target->tag == symbol::symbol_tag::GLOBAL) {
						handle target_global = reinterpret_cast<global*>(patch->target.get());
						bool is_tls = sections[target_global->parent_section].flags & module_section::TLS;

						*relocations_ptr++ = coff_image_relocation{
							.virtual_address = static_cast<u32>(actual_pos),
							.symbol_table_index = static_cast<u32>(symbol_id),
							.type = static_cast<u16>(is_tls ? IMAGE_REL_AMD64_SECREL : IMAGE_REL_AMD64_REL32),
						};
					}
					else {
						ASSERT(false, "not implemented");
					}
				}
			}

			for (const auto& global : section.globals) {
				for (const auto& object : global->objects) {
					u64 actual_pos = global->position + object.offset;

					if (object.type == init_object::RELOCATION) {
						*relocations_ptr++ = coff_image_relocation{
							.virtual_address = static_cast<u32>(actual_pos),
							.symbol_table_index = static_cast<u32>(object.relocation->symbol_id),
							.type = IMAGE_REL_AMD64_ADDR64,
						};
					}
				}
			}

			ASSERT(
				relocations_ptr - reinterpret_cast<coff_image_relocation*>(relocations.get_data()) == relocation_count, 
				"invalid relocation sequence detected"
			);

			out_file.append(relocations);
		}

		// write symbols
		u32 string_table_mark = 4;
		u64 string_table_length = 0;

		std::vector<std::string> string_table_data;
		string_table_data.resize(unique_id_counter);
		utility::byte_writer symbol_table_writer(header.symbol_count * sizeof(coff_symbol));

		u64 symbol_count = 1;

		for (auto& section : sections) {
			coff_symbol s = {
				.section_number = static_cast<i16>(symbol_count),
				.storage_class = IMAGE_SYM_CLASS_STATIC,
				.aux_symbols_count = 1
			};

			std::memcpy(s.short_name, section.name.data(), 8 * sizeof(char));
			symbol_table_writer.write(s);

			coff_auxiliary_section_symbol aux = {
				.length = section.total_size,
				.reloc_count = static_cast<u16>(section.relocation_count),
				.number = static_cast<i16>(symbol_count),
				.selection = static_cast<u8>(section.comdat.ty != comdat::NONE ? 2 : 0),
			};

			symbol_table_writer.write(aux);
			symbol_count++;
		}

		for (u64 i = 0; i < sections.size(); ++i) {
			i16 section_num = sections[i].section_index;

			if (handle<coff_unwind_info> unwind = unwinds[i]) {
				// .pdata
				// .xdata
				coff_symbol s[2] = {
					{ 
						.short_name = ".pdata",
						.section_number = static_cast<i16>(symbol_count),
						.storage_class = IMAGE_SYM_CLASS_STATIC,
						.aux_symbols_count = 1
					},
					{ 
						.short_name = ".xdata",
						.section_number = static_cast<i16>(symbol_count + 1),
						.storage_class = IMAGE_SYM_CLASS_STATIC,
						.aux_symbols_count = 1
					},
				};

				coff_auxiliary_section_symbol aux[2] = {
					{
						.length = static_cast<u32>(unwind->pdata_chunk.get_size()),
						.reloc_count = static_cast<u16>(unwind->patch_count),
						.number = static_cast<i16>(symbol_count)
					},
					{.
						length = static_cast<u32>(unwind->xdata_chunk.get_size()),
						.number = static_cast<i16>(symbol_count + 1)
					},
				};

				if (i > 0) {
					aux[0].selection = 5;
					aux[0].number = section_num;
				}

				symbol_table_writer.write(s[0]);
				symbol_table_writer.write(aux[0]);
				symbol_table_writer.write(s[1]);
				symbol_table_writer.write(aux[1]);
				symbol_count += 2;
			}
		}

		for (auto& section : sections) {
			i16 section_index = section.section_index;

			for (const auto& function : section.functions) {
				bool is_extern = function->parent->linkage == PUBLIC;
				coff_symbol sym = {
					.value = static_cast<u32>(function->code_position),
					.section_number = section_index,
					.storage_class = static_cast<u8>(is_extern ? IMAGE_SYM_CLASS_EXTERNAL : IMAGE_SYM_CLASS_STATIC)
				};

				std::string name = function->parent->symbol.name;
				u64 name_length = name.size();

				ASSERT(name_length < std::numeric_limits<u16>::max(), "invalid name");

				if (name_length >= 8) {
					sym.long_name[0] = 0; // this value is 0 for long names
					sym.long_name[1] = string_table_mark;

					string_table_data[string_table_length++] = name;
					string_table_mark += static_cast<u32>(name_length) + 1;
				}
				else {
					std::memcpy(sym.short_name, name.c_str(), name_length + 1);
				}

				symbol_table_writer.write(sym);
			}

			for (const auto& global : section.globals) {
				bool is_extern = global->linkage == PUBLIC;

				ASSERT(section_index == global->parent_section + 1, "invalid parent section");
				coff_symbol sym = {
					.value = global->position,
					.section_number = section_index,
					.storage_class = static_cast<u8>(is_extern ? IMAGE_SYM_CLASS_EXTERNAL : IMAGE_SYM_CLASS_STATIC)
				};

				if (global->symbol.name[0] != 0) {
					u64 name_len = global->symbol.name.size();
					ASSERT(name_len < std::numeric_limits<u16>::max(), "invalid name length");

					if (name_len >= 8) {
						sym.long_name[0] = 0; // this value is 0 for long names
						sym.long_name[1] = string_table_mark;

						string_table_data[string_table_length++] = global->symbol.name;
						string_table_mark += static_cast<u32>(name_len) + 1;
					}
					else {
						std::memcpy(sym.short_name, global->symbol.name.c_str(), name_len + 1);
					}
				}
				else {
					std::snprintf(reinterpret_cast<char*>(sym.short_name), 8, "$%06zx", global->symbol.symbol_id);
				}

				symbol_table_writer.write(sym);
			}
		}

		for (const auto& ex : externals) {
			coff_symbol sym = {
				.value = 0,
				.section_number = 0,
				.storage_class = IMAGE_SYM_CLASS_EXTERNAL
			};

			u64 name_length = ex->symbol.name.size();
			ASSERT(name_length < std::numeric_limits<u16>::max(), "invalid name length");

			if (name_length >= 8) {
				sym.long_name[0] = 0; // this value is 0 for long names
				sym.long_name[1] = string_table_mark;

				string_table_data[string_table_length++] = ex->symbol.name;
				string_table_mark += static_cast<u32>(name_length) + 1;
			}
			else {
				std::memcpy(sym.short_name, ex->symbol.name.c_str(), name_length + 1);
			}

			symbol_table_writer.write(sym);
		}

		ASSERT(symbol_table_writer.get_position() == symbol_table_writer.get_size(), "invalid write position");
		out_file.append(symbol_table_writer);

		utility::byte_buffer chunk = utility::byte_buffer::zero_initialize(string_table.size);
		std::memcpy(chunk.get_data(), &string_table_mark, sizeof(u32));
		u64 j = 4;

		for (u64 i = 0; i < string_table_length; ++i) {
			u64 l = string_table_data[i].size() + 1;
			memcpy(&chunk[j], string_table_data[i].c_str(), l);
			j += l;
		}

		out_file.append(chunk);
		return out_file;
	}

	auto coff_file_emitter::generate_unwind_info(module& module, u64 xdata_section, const module_section& section) -> handle<coff_unwind_info> {
		const u64 function_count = section.functions.size();
		const bool overflow = function_count * 3 >= 0xFFFF;
		handle unwind = static_cast<coff_unwind_info*>(module.m_allocator.allocate(sizeof(coff_unwind_info)));
	 
	 	unwind->patch_count = function_count * 3;
	 	unwind->section_num = (xdata_section + 1) * 2;
	 
	 	// generate .pdata
	 	unwind->pdata_chunk = utility::byte_buffer(function_count * 3 * sizeof(u32));
	 	const auto pdata = reinterpret_cast<uint32_t*>(unwind->pdata_chunk.get_data());
	 
	 	unwind->pdata_relocations = utility::byte_buffer((overflow + function_count * 3) * sizeof(coff_image_relocation));
	 	auto relocations = reinterpret_cast<coff_image_relocation*>(unwind->pdata_relocations.get_data());
	 
	 	if (overflow) {
	 		*relocations++ = coff_image_relocation{
	 			.virtual_address = static_cast<u32>(function_count * 3)
	 		};
	 	}
	 
	 	// generate .xdata
	 	utility::byte_buffer xdata;
	 
	 	for(u64 i = 0; i < section.functions.size(); ++i) {
	 		const handle<compiled_function> function = section.functions[i];
	 
	 		const u32 unwind_info = static_cast<u32>(xdata.get_size());
			emit_win_unwind_info(xdata, function, function->stack_usage);
	 
	 		// write .pdata
	 		const u32 position = static_cast<u32>(function->code_position);
	 		const u64 j = i * 3;

	 		pdata[j + 0] = position;
	 		pdata[j + 1] = position + static_cast<u32>(function->bytecode.get_size());
	 		pdata[j + 2] = unwind_info;
	 
	 		// .pdata has relocations
	 		const u32 sym = static_cast<u32>(function->parent->symbol.symbol_id);

	 		relocations[j + 0] = coff_image_relocation{
	 			.virtual_address = static_cast<u32>(j * 4),
	 			.symbol_table_index = sym,
	 			.type = IMAGE_REL_AMD64_ADDR32NB,
	 		};
	 
	 		relocations[j + 1] = coff_image_relocation{
	 			.virtual_address = static_cast<u32>(j * 4) + 4,
	 			.symbol_table_index = sym,
	 			.type = IMAGE_REL_AMD64_ADDR32NB
	 		};
	 
	 		relocations[j + 2] = coff_image_relocation{
	 			.virtual_address = static_cast<u32>(j * 4) + 8,
	 			.symbol_table_index = static_cast<u32>(unwind->section_num), // .xdata section
	 			.type = IMAGE_REL_AMD64_ADDR32NB
	 		};
	 	}
	 
	 	unwind->xdata_chunk = utility::byte_buffer(xdata.get_size());
	 	memcpy(unwind->xdata_chunk.get_data(), xdata.get_data(), xdata.get_size());
	 
	 	unwind->pdata_header = coff_section_header {
	 		.name = ".pdata",
	 		.raw_data_size = static_cast<u32>(unwind->pdata_chunk.get_size()),
	 		.relocation_count = static_cast<u16>(unwind->patch_count >= 0xFFFF ? 0xFFFF : unwind->patch_count),
	 		.characteristics = COFF_CHARACTERISTICS_RODATA | (unwind->patch_count >= 0xFFFF ? IMAGE_SCN_LNK_NRELOC_OVFL : 0),
	 	};
	 
	 	unwind->xdata_header = coff_section_header {
	 		.name = ".xdata",
	 		.raw_data_size = static_cast<u32>(unwind->xdata_chunk.get_size()),
	 		.characteristics = COFF_CHARACTERISTICS_RODATA,
	 	};

		return unwind;
	}

	void coff_file_emitter::emit_win_unwind_info(utility::byte_buffer& buffer, handle<compiled_function> function, u64 stack_usage) {
		const u64 patch_position = buffer.get_size();
		u8 code_count = 0;

 		const unwind_info unwind = {
 			.version = 1,
 			.flags = UNWIND_FLAG_EHANDLER,
 			.prolog_length = function->prologue_length,
 			.code_count = 0,
 			.frame_register = x64::RBP,
 			.frame_offset = 0,
 		};
	 
 		buffer.append_type(unwind);
	 
 		if (stack_usage == 8) {
 			// no real prologue
 		}
 		else {
 			const unwind_code codes[] = {
 				// sub rsp, stack_usage
				{ .offset = {.code_offset = 8, .unwind_op = unwind_op::ALLOC_SMALL, .op_info = static_cast<u8>(stack_usage / 8 - 1) }},
 				// mov rbp, rsp
				{.offset = {.code_offset = 4, .unwind_op = unwind_op::SET_FPREG, .op_info = 0 }},
 				// push rbp
				{.offset = {.code_offset = 1, .unwind_op = unwind_op::PUSH_NONVOL, .op_info = x64::RBP }},
 			};
	 
 			buffer.append_type(codes);
 			code_count += 3;
 		}
	 
 		buffer.patch_byte(patch_position + offsetof(unwind_info, code_count), code_count);
	}

	auto coff_file_emitter::layout_relocations(std::vector<module_section>& sections, u32 output_size, u32 relocation_size) -> u32 {
		// calculate the relocation layout
 		for(auto& section : sections) {
 			u32 relocation_count = 0;
	 
 			for(const auto& function : section.functions) {
 				relocation_count += emit_call_patches(function);
 			}
	 
 			for(const auto& global : section.globals) {
 				for(const auto& object : global->objects) {
 					relocation_count += object.type == init_object::RELOCATION;
 				}
 			}

 			section.relocation_count = relocation_count;
 			section.relocation_position = output_size;

			output_size += relocation_count * relocation_size;
 		}

 		return output_size;
	}

	auto coff_file_emitter::emit_call_patches(handle<compiled_function> compiled_func) -> u32 {
		const u64 source_section = compiled_func->parent->parent_section;
		u64 ordinal = 0;

		for (const handle<symbol_patch> patch : compiled_func->patches) {
		 	if (patch->target->tag == symbol::symbol_tag::FUNCTION) {
				const u64 destination_section = reinterpret_cast<function*>(patch->target.get())->output.parent->parent_section;
	 
		 		// you can't do relocations across sections
		 		if (source_section == destination_section) {
		 			ASSERT(patch->pos < compiled_func->bytecode.get_size(), "invalid patch position");
	 
		 			const u64 actual_position = compiled_func->code_position + patch->pos + 4;
		 			u32 position = static_cast<u32>(reinterpret_cast<function*>(patch->target.get())->output.code_position - actual_position);
		 			memcpy(&compiled_func->bytecode[patch->pos], &position, sizeof(uint32_t));
	 
		 			ordinal += 1;
		 			patch->internal = true;
		 		}
		 	}
		}
	 
		return static_cast<u32>(compiled_func->patches.get_size() - ordinal);
	}

	auto coff_file_emitter::helper_write_section(u64 write_pos, const module_section* section, u32 pos, utility::byte_buffer& buffer) -> u64 {
		ASSERT(write_pos == pos, "invalid write position");
		utility::byte* data = &buffer[pos];

		// place functions
		for (const auto& function : section->functions) {
			if (function != nullptr) {
				memcpy(data + function->code_position, function->bytecode.get_data(), function->bytecode.get_size());
			}
		}

		// place globals
		for (const auto& global : section->globals) {
			memset(&data[global->position], 0, global->size);

			for (const auto& object : global->objects) {
				if (object.type == init_object::REGION) {
					ASSERT(object.offset + object.r.size <= global->size, "invalid object layout");
					memcpy(&data[global->position + object.offset], object.r.ptr, object.r.size);
				}
			}
		}

		return write_pos + section->total_size;
	}
	auto coff_file_emitter::machine_to_coff_machine(target target) -> coff_machine::value {
		switch (target.get_arch()) {
			case arch::X64: return coff_machine::AMD64;
		}

		ASSERT(false, "coversion not implemented");
		return coff_machine::NONE;
	}
}
