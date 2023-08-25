#include "optional_header_64_bit.h"

namespace sigma::windows {
	void optional_header_64_bit::print() const {
		utility::console::out << "optional header (64 bit):\n";
		utility::console::out << "  magic:                   " << magic << '\n';
		utility::console::out << "  major_linker_version:    " << major_linker_version << '\n';
		utility::console::out << "  minor_linker_version:    " << minor_linker_version << '\n';
		utility::console::out << "  code_size:               " << code_size << '\n';
		utility::console::out << "  initialized_data_size:   " << initialized_data_size << '\n';
		utility::console::out << "  uninitialized_data_size: " << uninitialized_data_size << '\n';
		utility::console::out << "  entry_point_address:     " << entry_point_address << '\n';
		utility::console::out << "  code_base:               " << code_base << '\n';

		utility::console::out << "  image_base:              " << image_base << '\n';
		utility::console::out << "  section_alignment:       " << section_alignment << '\n';
		utility::console::out << "  file_alignment:          " << file_alignment << '\n';
		utility::console::out << "  major_os_version:        " << major_os_version << '\n';
		utility::console::out << "  minor_os_version:        " << minor_os_version << '\n';
		utility::console::out << "  major_image_version:     " << major_image_version << '\n';
		utility::console::out << "  minor_image_version:     " << minor_image_version << '\n';
		utility::console::out << "  major_subsystem_version: " << major_subsystem_version << '\n';
		utility::console::out << "  minor_subsystem_version: " << minor_subsystem_version << '\n';
		utility::console::out << "  win32_version_value:     " << win32_version_value << '\n';
		utility::console::out << "  image_size:              " << image_size << '\n';
		utility::console::out << "  header_size:             " << header_size << '\n';
		utility::console::out << "  check_sum:               " << check_sum << '\n';
		utility::console::out << "  subsystem:               " << subsystem << '\n';
		utility::console::out << "  DLL_characteristics:     " << DLL_characteristics << '\n';
		utility::console::out << "  stack_reserve_size:      " << stack_reserve_size << '\n';
		utility::console::out << "  stack_commit_size:       " << stack_commit_size << '\n';
		utility::console::out << "  heap_reserve_size:       " << heap_reserve_size << '\n';
		utility::console::out << "  heap_commit_size:        " << heap_commit_size << '\n';
		utility::console::out << "  loader_flags:            " << loader_flags << '\n';
		utility::console::out << "  RVA_and_size_count:      " << RVA_and_size_count << '\n';

		utility::console::out << "  data_directories:\n";
		for (u64 i = 0; i < 16; i++) {
			utility::console::out
				<< "    virtual address: " << data_directories[i].virtual_address
				<< ", size: " << data_directories[i].size << '\n';
		}
	}
}