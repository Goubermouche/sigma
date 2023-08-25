#include "section_header.h"

namespace sigma::windows {
	void section_header::print() const {
		utility::console::out << "section header:\n";
		utility::console::out << "  name:                " << std::string(name, name + 8) << '\n';
		utility::console::out << "  virtual_size:        " << virtual_size << '\n';
		utility::console::out << "  virtual_address:     " << virtual_address << '\n';
		utility::console::out << "  raw_data_size:       " << raw_data_size << '\n';
		utility::console::out << "  raw_data_pointer:    " << raw_data_pointer << '\n';
		utility::console::out << "  relocation_pointer:  " << relocation_pointer << '\n';
		utility::console::out << "  line_number_pointer: " << line_number_pointer << '\n';
		utility::console::out << "  relocation_count:    " << relocation_count << '\n';
		utility::console::out << "  line_number_count:   " << line_number_count << '\n';
		utility::console::out << "  characteristics:     " << (u32)characteristics << '\n';
	}

	section_header_characteristics operator|(
		section_header_characteristics lhs, 
		section_header_characteristics rhs
	) {
		return static_cast<section_header_characteristics>(
			static_cast<u16>(lhs) | static_cast<u16>(rhs)
		);
	}

	section_header_characteristics operator&(
		section_header_characteristics lhs, 
		section_header_characteristics rhs
	) {
		return static_cast<section_header_characteristics>(
			static_cast<u16>(lhs) & static_cast<u16>(rhs)
		);
	}

	bool operator!=(section_header_characteristics lhs, u32 rhs) {
		return static_cast<u32>(lhs) != rhs;
	}

	bool operator==(section_header_characteristics lhs, u32 rhs) {
		return static_cast<u32>(lhs) == rhs;
	}
}