#include "file_header.h"
#include <windows.h>

using namespace utility::types;

namespace sigma::windows {
	file_header_characteristics operator|(
		file_header_characteristics lhs,
		file_header_characteristics rhs
	) {
		return static_cast<file_header_characteristics>(
			static_cast<u16>(lhs) | static_cast<u16>(rhs)
		);
	}

	file_header_characteristics operator&(
		file_header_characteristics lhs,
		file_header_characteristics rhs
	) {
		return static_cast<file_header_characteristics>(
			static_cast<u16>(lhs) & static_cast<u16>(rhs)
		);
	}

	file_header_machine get_machine_type() {
#if defined(_M_IX86)
		return file_header_machine::i386;
#elif defined(_M_AMD64)
		return file_header_machine::amd64;
#elif defined(_M_ARM)
		return file_header_machine::arm;
#elif defined(_M_ARM64)
		return file_header_machine::arm64;
#elif defined(_M_IA64)
		return file_header_machine::ia64;
#elif defined(_M_MIPS)
		return file_header_machine::mips16; // bit of a guess
#elif defined(_M_PPC)
		return file_header_machine::powerpc;
#else
		// todo: cover all the cases 
		return file_header_machine::unknown;
#endif
	}

	void file_header::print() const {
		utility::console::out << "file header:\n";
		utility::console::out << "  machine:                 " << (u16)machine << '\n';
		utility::console::out << "  section_count:           " << section_count << '\n';
		utility::console::out << "  time_date_stamp:         " << time_date_stamp << '\n';
		utility::console::out << "  pointer_to_symbol_table: " << pointer_to_symbol_table << '\n';
		utility::console::out << "  symbol_count:            " << symbol_count << '\n';
		utility::console::out << "  optional_header_size:    " << optional_header_size << '\n';
		utility::console::out << "  characteristics:         " << (u16)characteristics << '\n';
	}
}