#include "coff.h"
#include "intermediate_representation/target/system/win/win.h"
#include "intermediate_representation/module.h"

namespace sigma::ir {
	utility::byte_buffer coff_file_emitter::emit(module& module) {
		std::vector<handle<external>> externals = module.generate_externals();
		auto output = module.get_output();
		u64 section_count = output.sections.size();
		std::vector<handle<coff_unwind_info>> unwinds(section_count);

		// mark correct flags on sections
		for (module_section& section : output.sections) {
			u32 flags = coff::SECTION_READ;

			flags |= section.flags & module_section::WRITE ? coff::SECTION_WRITE : 0;
			flags |= section.flags & module_section::EXEC  ? coff::SECTION_EXECUTE | coff::SECTION_CODE : coff::SECTION_INIT;

			if (section.com.ty != comdat::NONE) {
				flags |= coff::SECTION_COMDAT;
			}

			section.export_flags = flags;
		}

		// generate unique ID's
		u64 unique_id_counter = section_count * 2;

		for (module_section& section : output.sections) {
			if (!section.functions.empty()) {
				unique_id_counter += 4;
			}
		}

		for (module_section& section : output.sections) {
			for (const handle<compiled_function> function : section.functions) {
				function->parent->symbol.id = unique_id_counter++;
			}

			for (const handle<global> global : section.globals) {
				global->symbol.id = unique_id_counter++;
			}
		}

		for (handle<external> external : externals) {
			external->symbol.id = unique_id_counter++;
		}

		for (u64 i = 0; i < output.sections.size(); ++i) {
			output.sections[i].section_index = static_cast<u8>(i + 1);

			// generate unwind information for each section with functions
			if (!output.sections[i].functions.empty()) {
				unwinds[i] = generate_unwind_info(module, section_count, output.sections[i]);
				section_count += 2; // .pdata + .xdata
			}
		}

		// COFF file header & section headers
		coff_file_header header = {
			.section_count = static_cast<u16>(section_count),
			.timestamp = 1056582000u,
			.symbol_table = 0,
			.symbol_count = static_cast<u32>(unique_id_counter),
			.flags = IMAGE_FILE_LINE_NUMS_STRIPPED
		};

		header.machine = machine_to_coff_machine(module.get_target());

		// calculate output size & layout object
		u32 output_size = sizeof(coff_file_header);
		output_size += static_cast<u32>(section_count) * sizeof(coff_section_header);

		for (u64 i = 0; i < output.sections.size(); ++i) {
			output.sections[i].raw_data_pos = output_size;
			output_size += output.sections[i].total_size;

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
		output_size = layout_relocations(output.sections, output_size, sizeof(coff_image_relocation));

		header.symbol_table = output_size;
		output_size += header.symbol_count * sizeof(coff_symbol);

		// first 4 bytes of the table are the size of the table
		string_table string_table = {
			.pos = output_size,
			.size = 4
		};

		// compute string table size
		for (const module_section& section : output.sections) {
			for (const handle<compiled_function> function : section.functions) {
				u64 name_length = function->parent->symbol.name.size() + 1;

				if (name_length >= 8) {
					string_table.size += static_cast<u32>(name_length) + 1;
				}
			}

			for (const handle<global> global : section.globals) {
				u64 name_length = global->symbol.name.size() + 1;

				if (name_length >= 8) {
					string_table.size += static_cast<u32>(name_length) + 1;
				}
			}
		}

		for (const handle<external> external : externals) {
			u64 name_length = external->symbol.name.size() + 1;

			if (name_length >= 8) {
				string_table.size += static_cast<u32>(name_length) + 1;
			}
		}

		output_size += string_table.size;

		// write the file
		utility::byte_buffer out_file;
		utility::byte_buffer headers = utility::byte_buffer::create_zero(sizeof(coff_file_header) + sizeof(coff_section_header) * section_count);

		// write the file header 
		auto* file = reinterpret_cast<coff_file_header*>(headers.get_data());
		*file = header;

		// write sections headers
		auto sec_headers = reinterpret_cast<coff_section_header*>(file + 1);

		for (module_section& section : output.sections) {
			coff_section_header section_header = {
				.raw_data_size = section.total_size,
				.raw_data_pos = section.raw_data_pos,
				.relocation_pointer = section.relocation_position,
				.characteristics = section.export_flags
			};

			u64 section_name_length = section.name.size() + 1;
			std::memcpy(section_header.name, section.name.c_str(), section_name_length > 8 ? 8 : section_name_length);

			if (section.relocation_count >= 0xFFFF) {
				section_header.relocation_count = 0xFFFF;
				section_header.characteristics |= IMAGE_SCN_LNK_NRELOC_OVFL;
			}
			else {
				section_header.relocation_count = static_cast<u16>(section.relocation_count);
			}

			*sec_headers++ = section_header;
		}

		for (u64 i = 0; i < output.sections.size(); ++i) {
			if (unwinds[i]) {
				memcpy(sec_headers + 0, &unwinds[i]->pdata_header, sizeof(coff_section_header));
				memcpy(sec_headers + 1, &unwinds[i]->xdata_header, sizeof(coff_section_header));
				sec_headers += 2;
			}
		}

		out_file.append(headers);

		for (u64 i = 0; i < output.sections.size(); ++i) {
			utility::byte_buffer sec = utility::byte_buffer::create_zero(output.sections[i].total_size);
			helper_write_section(0, &output.sections[i], 0, sec);
			out_file.append(sec);

			if (handle<coff_unwind_info> unwind = unwinds[i]) {
				out_file.append(unwind->pdata_chunk);
				out_file.append(unwind->xdata_chunk);
				out_file.append(unwind->pdata_relocations);
			}
		}

		// write relocations
		for (module_section& section : output.sections) {
			u32 relocation_count = section.relocation_count;
			utility::byte_buffer relocations(relocation_count * sizeof(coff_image_relocation));
			relocations.zero_fill();

			auto relocations_ptr = reinterpret_cast<coff_image_relocation*>(relocations.get_data());

			for (const handle<compiled_function> function : section.functions) {
				u64 source_offset = function->code_position;

				for (handle<symbol_patch> patch = function->first_patch; patch; patch = patch->next) {
					if (patch->internal) {
						continue;
					}

					u64 actual_pos = source_offset + patch->pos;
					u64 symbol_id = patch->target->id;
					ASSERT(symbol_id != 0, "invalid symol ID");

					if (patch->target->type == symbol::symbol_type::FUNCTION || patch->target->type == symbol::symbol_type::EXTERNAL) {
						*relocations_ptr++ = coff_image_relocation{
							.virtual_address = static_cast<u32>(actual_pos),
							.symbol_table_index = static_cast<u32>(symbol_id),
							.type = IMAGE_REL_AMD64_REL32,
						};
					}
					else if (patch->target->type == symbol::symbol_type::GLOBAL) {
						handle target_global = reinterpret_cast<global*>(patch->target.get());
						bool is_tls = output.sections[target_global->parent_section].flags & module_section::TLS;

						*relocations_ptr++ = coff_image_relocation{
							.virtual_address = static_cast<u32>(actual_pos),
							.symbol_table_index = static_cast<u32>(symbol_id),
							.type = static_cast<u16>(is_tls ? IMAGE_REL_AMD64_SECREL : IMAGE_REL_AMD64_REL32),
						};
					}
					else {
						NOT_IMPLEMENTED();
					}
				}
			}

			for (const handle<global> global : section.globals) {
				for (const init_object& object : global->objects) {
					u64 actual_pos = global->position + object.offset;

					if(object.type == init_object::RELOCATION) {
						*relocations_ptr++ = coff_image_relocation{
							.virtual_address = static_cast<u32>(actual_pos),
							.symbol_table_index = static_cast<u32>(object.relocation->id),
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
		utility::byte_buffer symbol_table_writer = utility::byte_buffer::create_reserve(header.symbol_count * sizeof(coff_symbol));
		u64 symbol_count = 1;

		for (module_section& section : output.sections) {
			coff_symbol symbol = {
				.section_number = static_cast<i16>(symbol_count),
				.storage_class = IMAGE_SYM_CLASS_STATIC,
				.aux_symbols_count = 1
			};

			std::memcpy(symbol.short_name, section.name.c_str(), 8 * sizeof(char));
			symbol_table_writer.append_type(symbol);

			coff_auxiliary_section_symbol aux = {
				.length = section.total_size,
				.relocation_count = static_cast<u16>(section.relocation_count),
				.number = static_cast<i16>(symbol_count),
				.selection = static_cast<u8>(section.com.ty != comdat::NONE ? 2 : 0),
			};

			symbol_table_writer.append_type(aux);
			symbol_count++;
		}

		for (u64 i = 0; i < output.sections.size(); ++i) {
			if (handle<coff_unwind_info> unwind = unwinds[i]) {
				// .pdata
				// .xdata
				coff_symbol symbols[2] = {
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

				coff_auxiliary_section_symbol auxiliary_symbols[2] = {
					{
						.length = static_cast<u32>(unwind->pdata_chunk.get_size()),
						.relocation_count = static_cast<u16>(unwind->patch_count),
						.number = static_cast<i16>(symbol_count)
					},
					{.
						length = static_cast<u32>(unwind->xdata_chunk.get_size()),
						.number = static_cast<i16>(symbol_count + 1)
					},
				};

				if (i > 0) {
					auxiliary_symbols[0].selection = 5;
					auxiliary_symbols[0].number = static_cast<i16>(output.sections[i].section_index);
				}

				symbol_table_writer.append_type(symbols[0]);
				symbol_table_writer.append_type(auxiliary_symbols[0]);
				symbol_table_writer.append_type(symbols[1]);
				symbol_table_writer.append_type(auxiliary_symbols[1]);
				symbol_count += 2;
			}
		}

		for (module_section& section : output.sections) {
			i16 section_index = static_cast<i16>(section.section_index);

			for (const handle<compiled_function> function : section.functions) {
				bool is_extern = function->parent->symbol.link == linkage::PUBLIC;
				coff_symbol sym = {
					.value = static_cast<u32>(function->code_position),
					.section_number = section_index,
					.storage_class = static_cast<u8>(is_extern ? IMAGE_SYM_CLASS_EXTERNAL : IMAGE_SYM_CLASS_STATIC)
				};

				std::string name = function->parent->symbol.name;
				u64 name_length = name.size() + 1;

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

				symbol_table_writer.append_type(sym);
			}

			for (const handle<global> global : section.globals) {
				bool is_extern = global->symbol.link == linkage::PUBLIC;

				ASSERT(section_index == global->parent_section + 1, "invalid parent section");
				coff_symbol sym = {
					.value = global->position,
					.section_number = section_index,
					.storage_class = static_cast<u8>(is_extern ? IMAGE_SYM_CLASS_EXTERNAL : IMAGE_SYM_CLASS_STATIC)
				};

				if (global->symbol.name[0] != 0) {
					u64 name_len = global->symbol.name.size() + 1;
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
					ASSERT(std::snprintf(reinterpret_cast<char*>(sym.short_name), 8, "$%06zx", global->symbol.id) > 0, "invalid snprintf call");
				}

				symbol_table_writer.append_type(sym);
			}
		}

		for (const handle<external> external : externals) {
			coff_symbol sym = {
				.value = 0,
				.section_number = 0,
				.storage_class = IMAGE_SYM_CLASS_EXTERNAL
			};

			u64 name_length = external->symbol.name.size() + 1;
			ASSERT(name_length < std::numeric_limits<u16>::max(), "invalid name length");

			if (name_length >= 8) {
				sym.long_name[0] = 0; // this value is 0 for long names
				sym.long_name[1] = string_table_mark;

				string_table_data[string_table_length++] = external->symbol.name;
				string_table_mark += static_cast<u32>(name_length);
			}
			else {
				std::memcpy(sym.short_name, external->symbol.name.c_str(), name_length);
			}

			symbol_table_writer.append_type(sym);
		}

		out_file.append(symbol_table_writer);

		utility::byte_buffer chunk = utility::byte_buffer::create_zero(string_table.size);
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
	 		const u32 sym = static_cast<u32>(function->parent->symbol.id);

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
 			.frame_register = static_cast<u16>(x64::gpr::RBP),
 			.frame_offset = 0,
 		};
	 
 		buffer.append_type(unwind);
	 
 		if (stack_usage == 8) {
 			// no real prologue
 		}
 		else {
 			const unwind_code codes[] = {
 				// sub rsp, stack_usage
				{ .o = {.code_offset = 8, .unwind_op = unwind_op::ALLOC_SMALL, .op_info = static_cast<u8>(stack_usage / 8 - 1) }},
 				// mov rbp, rsp
				{.o = {.code_offset = 4, .unwind_op = unwind_op::SET_FPREG, .op_info = 0 }},
 				// push rbp
				{.o = {.code_offset = 1, .unwind_op = unwind_op::PUSH_NONVOL, .op_info = static_cast<u8>(x64::gpr::RBP)}},
 			};
	 
 			buffer.append_type(codes);
 			code_count += 3;
 		}
	 
 		buffer.patch_byte(patch_position + offsetof(unwind_info, code_count), code_count);
	}

	auto coff_file_emitter::machine_to_coff_machine(target target) -> coff_machine::value {
		switch (target.get_arch()) {
			case arch::X64: return coff_machine::AMD64;
		}

		NOT_IMPLEMENTED();
		return coff_machine::NONE;
	}
} // namespace sigma::ir
