#include "code_generator.h"

#include <Windows.h>

namespace sigma {
	//utility::outcome::result<void> context::compile(
	//	const filepath& path
	//) {
	//	std::vector<unsigned char> text_bytecode = {
	//		0xC3
	//	};

	//	pe_builder builder;
	//	builder.add_section(
	//		".text",
	//		text_bytecode,
	//		windows::section_header_characteristics::mem_execute | 
	//		windows::section_header_characteristics::mem_read |
	//		windows::section_header_characteristics::cnt_code |
	//		windows::section_header_characteristics::align_16_bytes
	//	);

	//	 //std::string msg = "Hello world!";
	//	 //std::vector<unsigned char> rdata_bytecode(msg.begin(), msg.end());
	//	 //
	//	 //builder.add_section(
 //		//	".rdata",
 //		//	rdata_bytecode,
 //		//	windows::section_header_characteristics::cnt_initialized_data |
 //		//	windows::section_header_characteristics::mem_read |
 //		//	windows::section_header_characteristics::align_16_bytes
 //		//);

	//	import_table table;
	//	table.add_import(
	//		"kernel32.dll",
	//		{ "WriteConsoleA", "GetStdHandle" },
	//		builder.get_current_RVA()
	//	);

	//	builder.add_section(
	//		".idata",
	//		table.get_bytecode(),
	//		windows::section_header_characteristics::cnt_initialized_data |
	//		windows::section_header_characteristics::mem_read |
	//		windows::section_header_characteristics::mem_write |
	//		windows::section_header_characteristics::align_16_bytes
	//	);

	//	builder.emit_to_file(path);

	//	return utility::outcome::success();
	//}

	//u64 align(u64 value, u64 alignment) {
	//	return (value + alignment - 1) & ~(alignment - 1);
	//}

	//pe_builder::pe_builder() {
	//	// initialize the dos header
	//	m_dos_header.magic = 0x5A4D;
	//	m_dos_header.cparhdr = 4;
	//	m_dos_header.lfanew = sizeof(windows::dos_header); // place the NT headers directly behind the DOS header

	//	// NT headers
	//	// technically, the PE signature goes here

	//	// initialize the file header
	//	m_file_header.machine = windows::get_machine_type();
	//	m_file_header.section_count = 0;
	//	m_file_header.optional_header_size = sizeof(windows::optional_header_64_bit);
	//	m_file_header.characteristics = windows::file_header_characteristics::executable;

	//	// initialize the optional header
	//	m_optional_header.magic = 0x020B; // PE32+
	//	m_optional_header.section_alignment = 4096;
	//	m_optional_header.file_alignment = 512;
	//	m_optional_header.major_linker_version = 14;
	//	m_optional_header.minor_linker_version = 0;
	//	m_optional_header.major_os_version = 6; // win10 compatibility
	//	m_optional_header.minor_os_version = 0;
	//	m_optional_header.major_image_version = 1;
	//	m_optional_header.major_subsystem_version = 6; // win10 compatibility
	//	m_optional_header.image_base = 0x140000000;
	//	m_optional_header.win32_version_value = 0;
	//	m_optional_header.subsystem = 3; // windows console subsystem
	//	m_optional_header.DLL_characteristics = 0x8160; // NX-Compatible and Terminal Server Aware
	//	m_optional_header.stack_reserve_size = 0x100000;
	//	m_optional_header.stack_commit_size = 0x1000;
	//	m_optional_header.heap_reserve_size = 0x100000;
	//	m_optional_header.heap_commit_size = 0x1000;
	//	m_optional_header.loader_flags = 0;
	//	m_optional_header.RVA_and_size_count = 0;

	//	u64 header_size =
	//		sizeof(windows::dos_header) +
	//		sizeof(g_pe_signature) +
	//		sizeof(windows::file_header) +
	//		sizeof(windows::optional_header_64_bit);

	//	m_optional_header.entry_point_address = align(header_size, m_optional_header.section_alignment);
	//	m_optional_header.code_base = m_optional_header.entry_point_address;
	//}

	//void pe_builder::add_section(
	//	const std::string& name,
	//	std::vector<unsigned char> data,
	//	windows::section_header_characteristics characteristics
	//) {
	//	m_file_header.section_count++;

	//	// If it's a code section, update the SizeOfCode in the optional header
	//	if ((characteristics & windows::section_header_characteristics::cnt_code) != 0) {
	//		m_optional_header.code_size += data.size();
	//	}

	//	windows::section_header new_section = {};
	//	memcpy(new_section.name, name.c_str(), std::min<size_t>(name.size(), 8));

	//	new_section.virtual_size = align(data.size(), m_optional_header.section_alignment);
	//	new_section.raw_data_size = align(data.size(), m_optional_header.file_alignment);

	//	if (m_section_data.empty()) {
	//		new_section.raw_data_pointer = align(
	//			sizeof(windows::dos_header) +
	//			sizeof(g_pe_signature) +
	//			sizeof(windows::file_header) +
	//			sizeof(windows::optional_header_64_bit) +
	//			sizeof(windows::section_header) * m_file_header.section_count,
	//			m_optional_header.file_alignment
	//		);
	//		new_section.virtual_address = align(m_optional_header.entry_point_address, m_optional_header.section_alignment);
	//	}
	//	else {
	//		const auto& last_section_header = m_sections.back();
	//		new_section.raw_data_pointer = last_section_header.raw_data_pointer +
	//			align(m_section_data.back().size(), m_optional_header.file_alignment);
	//		new_section.virtual_address = last_section_header.virtual_address +
	//			align(m_section_data.back().size(), m_optional_header.section_alignment);
	//	}

	//	if (name == ".idata") {
	//		m_optional_header.RVA_and_size_count = 2;
	//		m_optional_header.data_directories[IMAGE_DIRECTORY_ENTRY_IMPORT].virtual_address = new_section.virtual_address;
	//		m_optional_header.data_directories[IMAGE_DIRECTORY_ENTRY_IMPORT].size = static_cast<u32>(data.size());
	//	}

	//	new_section.characteristics = characteristics;
	//	m_sections.push_back(new_section);
	//	m_section_data.push_back(data);

	//	m_current_RVA = new_section.virtual_address + new_section.virtual_size;
	//}

	//u64 pe_builder::get_current_RVA() const {
	//	return m_current_RVA;
	//}

	//void pe_builder::emit_to_file(
	//	const filepath& path
	//) {
	//	std::ofstream file(path, std::ios::binary);

	//	if (!file.is_open()) {
	//		// return false; // Failed to open the file
	//	}

	//	file.write(reinterpret_cast<char*>(&m_dos_header), sizeof(m_dos_header));
	//	file.write(reinterpret_cast<const char*>(&g_pe_signature), sizeof(g_pe_signature));
	//	file.write(reinterpret_cast<char*>(&m_file_header), sizeof(m_file_header));

	//	u64 imageSize = align(
	//		sizeof(windows::dos_header) +
	//		sizeof(g_pe_signature) +
	//		sizeof(windows::file_header) +
	//		sizeof(windows::optional_header_64_bit) +
	//		m_sections.size() * sizeof(windows::section_header),
	//		m_optional_header.file_alignment
	//	);

	//	for (const auto& section : m_sections) {
	//		imageSize += align(section.virtual_size, m_optional_header.section_alignment);
	//	}

	//	m_optional_header.header_size =
	//		align(sizeof(windows::dos_header) +
	//			sizeof(g_pe_signature) +
	//			sizeof(windows::file_header) +
	//			sizeof(windows::optional_header_64_bit) + 
	//			m_sections.size() * sizeof(windows::section_header),
	//			m_optional_header.file_alignment);

	//	// Update the optional header's ImageSize field
	//	m_optional_header.image_size = align(imageSize, m_optional_header.section_alignment);

	//	file.write(reinterpret_cast<char*>(&m_optional_header), sizeof(m_optional_header));

	//	u64 section_size =
	//		sizeof(windows::dos_header) +
	//		sizeof(g_pe_signature) +
	//		sizeof(windows::file_header) +
	//		sizeof(windows::optional_header_64_bit) +
	//		m_sections.size() * sizeof(windows::section_header);

	//	for(const auto& section : m_sections) {
	//		file.write(reinterpret_cast<const char*>(&section), sizeof(section));
	//	}

	//	const u64 padding_size_headers = align(section_size, m_optional_header.file_alignment) - section_size;
	//	for(u64 i = 0; i < padding_size_headers; i++) {
	//		file.put(0);
	//	}

	//	for(u64 i = 0; i < m_sections.size(); i++) {
	//		file.write(reinterpret_cast<const char*>(m_section_data[i].data()), m_section_data[i].size());
	//		const u64 padding_size_section = align(m_section_data[i].size(), m_optional_header.file_alignment) - m_section_data[i].size();

	//		for (u64 j = 0; j < padding_size_section; j++) {
	//			file.put(0);
	//		}
	//	}

	//	file.close();

	//	m_dos_header.print();
	//	m_file_header.print();
	//	m_optional_header.print();
	//	for(const auto& section : m_sections) {
	//		section.print();
	//	}
	//}

	//// .idata
	//// IMAGE_IMPORT_DESCRIPTOR
	////     OriginalFirstThunk: RVA to IMAGE_THUNK_DATA #1
	////     Name:               RVA pointing to "kernel32.dll"
	////     FirstThunk:         ???
	//// IMAGE_IMPORT_DESCRIPTOR
	////     zero terminated
	//// ---- ILT ----
	//// IMAGE_THUNK_DATA #1
	////     ForwarderString:    RVA to IMAGE_IMPORT_BY_NAME #1
	//// IMAGE_THUNK_DATA #2
	////     ForwarderString:    RVA to IMAGE_IMPORT_BY_NAME #2
	//// IMAGE_THUNK_DATA #3
	////     zero terminated
	//// ---- function names ----
	//// IMAGE_IMPORT_BY_NAME #1 
	////     Name:               "WriteConsoleA"
	//// IMAGE_IMPORT_BY_NAME #2
	////     Name:               "GetStdHandle"
	//// ---- DLL names ----
	//// "kernel32.dll"
	//// ---- IAT ----
	//// 
	//// (padding to FileAlignment - 512B)

	//void import_table::add_import(
	//	const std::string& dll_name,
	//	const std::vector<std::string>& imports,
	//	u64 start_RVA
	//) {
	//	IMAGE_IMPORT_DESCRIPTOR descriptor = {};
	//	const IMAGE_IMPORT_DESCRIPTOR zero_descriptor = {};

	//	std::vector ILT(imports.size() + 1, IMAGE_THUNK_DATA());
	//	std::vector<ptr<import_by_name>> function_names;
	//	std::vector IAT(imports.size() + 1, IMAGE_THUNK_DATA());

	//	// init the function names
	//	for (auto& import : imports) {
	//		function_names.push_back(
	//			std::make_shared<import_by_name>(import)
	//		);
	//	}

	//	const u64 function_names_RVA =
	//		start_RVA +
	//		sizeof(IMAGE_IMPORT_DESCRIPTOR) * 2 +
	//		sizeof(IMAGE_THUNK_DATA) * ILT.size();

	//	std::vector<unsigned char> function_name_bytes;
	//	std::vector<u64> function_name_RVAs;

	//	for(const auto& import : function_names) {
	//		WORD hint = import->get_value()->Hint;
	//		function_name_bytes.insert(function_name_bytes.end(), reinterpret_cast<unsigned char*>(&hint), reinterpret_cast<unsigned char*>(&hint) + sizeof(WORD));
	//		function_name_RVAs.push_back(function_name_bytes.size() + function_names_RVA);
	//		char* name = reinterpret_cast<char*>(import->get_value()->Name);
	//		function_name_bytes.insert(function_name_bytes.end(), name, name + std::strlen(name) + 1);
	//	}

	//	// init the ILT
	//	// skip the last element since we need to zero terminate it 
	//	for (u64 i = 0; i < ILT.size() - 1; ++i) {
	//		ILT[i].u1.ForwarderString = function_name_RVAs[i];
	//	}

	//	descriptor.Name = function_name_RVAs.back() + std::strlen(imports.back().c_str()) + 1;
	//	descriptor.OriginalFirstThunk = start_RVA + sizeof(IMAGE_IMPORT_DESCRIPTOR) * 2;
	//	descriptor.FirstThunk = descriptor.Name + std::strlen(imports.back().c_str()) + 1;

	//	// serialize
	//	m_bytecode.append_range(serialize(descriptor));
	//	m_bytecode.append_range(serialize(zero_descriptor));

	//	// ILT
	//	for(const auto& element : ILT) {
	//		m_bytecode.append_range(serialize(element));
	//	}

	//	// function names
	//	m_bytecode.append_range(function_name_bytes);

	//	// DLL names
	//	m_bytecode.append_range(dll_name);

	//	// IAT
	//	for (const auto& element : IAT) {
	//		m_bytecode.append_range(serialize(element));
	//	}
	//}

	//const std::vector<unsigned char>& import_table::get_bytecode() const {
	//	return m_bytecode;
	//}

	//import_by_name::import_by_name(const std::string& name) {
	//	const u64 total_size = sizeof(WORD) + name.length() + 1;
	//	BYTE* buffer = new BYTE[total_size];
	//	memset(buffer, 0, total_size);

	//	m_value = ptr<IMAGE_IMPORT_BY_NAME>(
	//		reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(buffer),
	//		[](IMAGE_IMPORT_BY_NAME* ptr) {
	//			delete[] reinterpret_cast<BYTE*>(ptr);
	//		});

	//	m_value->Hint = 0;
	//	std::strcpy((char*)m_value->Name, name.c_str());
	//}

	//ptr<IMAGE_IMPORT_BY_NAME> import_by_name::get_value() const {
	//	return m_value;
	//}

	//code_generator::code_generator(
	//	const target& target
	//) {
	//	m_backend = std::make_unique<amd_backend>();
	//}

	//void code_generator::add_instruction(
	//	const instruction& instruction
	//) {
	//	auto code = m_backend->generate_code(instruction);
	//	m_code.append_range(code);
	//}

	//std::vector<utility::byte> code_generator::get_code() const {
	//	return m_code;
	//}

	//std::vector<utility::byte> amd_backend::generate_code(
	//	const instruction& instruction
	//) {
	//	return std::visit(*this, instruction);
	//}

	//std::vector<utility::byte> amd_backend::operator()(
	//	const mov& mov
	//) const	{
	//	// Error BaseEmitter::_emitI(InstId instId, const Operand_ & o0, const Operand_ & o1) {
	//	// 	return _emit(instId, o0, o1, noExt[2], noExt);
	//	// }



	//	return {};
	//}

	//std::vector<utility::byte> amd_backend::operator()(
	//	const ret& ret
	//) const {
	//	return { 0xC3 };
	//}

	//mov::mov(
	//	const operand& source, 
	//	const operand& destination
	//) : m_source(source),
	//m_destination(destination) {}


	code_generator::code_generator(const ir::builder& builder)
		: m_builder(builder) {

	}

	void code_generator::allocate_registers() {

	}
}
