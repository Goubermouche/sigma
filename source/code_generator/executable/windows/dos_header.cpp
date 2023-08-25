#include "dos_header.h"

namespace sigma::windows {
	void dos_header::print() const {
		utility::console::out << "dos header:\n";
		utility::console::out << "  magic:    " << magic << '\n';
		utility::console::out << "  cblp:     " << cblp << '\n';
		utility::console::out << "  cp:       " << cp << '\n';
		utility::console::out << "  crlc:     " << crlc << '\n';
		utility::console::out << "  cparhdr:  " << cparhdr << '\n';
		utility::console::out << "  minalloc: " << minalloc << '\n';
		utility::console::out << "  maxalloc: " << maxalloc << '\n';
		utility::console::out << "  ss:       " << ss << '\n';
		utility::console::out << "  sp:       " << sp << '\n';
		utility::console::out << "  csum:     " << csum << '\n';
		utility::console::out << "  ip:       " << ip << '\n';
		utility::console::out << "  cs:       " << cs << '\n';
		utility::console::out << "  lfarlc:   " << lfarlc << '\n';
		utility::console::out << "  ovno:     " << ovno << '\n';

		utility::console::out << "  res:      ";
		for (u64 i = 0; i < 4; i++) {
			utility::console::out << res[i] << ' ';
		}

		utility::console::out << "\n  oemid:    " << oemid << '\n';
		utility::console::out << "  oeminfo:  " << oeminfo << '\n';

		utility::console::out << "  res2:     ";
		for (u64 i = 0; i < 10; i++) {
			utility::console::out << res2[i] << ' ';
		}

		utility::console::out << "\n  lfanew:   " << lfanew << '\n';
	}
}