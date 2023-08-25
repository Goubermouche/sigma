#pragma once
#include <utility/diagnostics/console.h>

using namespace utility::types;

namespace sigma::windows {
	struct dos_header {
		void print() const;

		u16 magic;    // magic number
		u16 cblp;     // bytes on last page file
		u16 cp;       // pages in file
		u16 crlc;     // relocations
		u16 cparhdr;  // size of header in paragraphs
		u16 minalloc; // minimum extra paragraphs needed
		u16 maxalloc; // maximum extra paragraphs needed
		u16 ss;       // initial (relative) SS value
		u16 sp;       // initial SP value
		u16 csum;     // checksum
		u16 ip;       // initial IP value
		u16 cs;       // initial (relative) CS value
		u16 lfarlc;   // file address of relocation table
		u16 ovno;     // overlay number
		u16 res[4];   // reserved words
		u16 oemid;    // OEM identifier (for oeminfo)
		u16 oeminfo;  // OEM information; oemid specific
		u16 res2[10]; // reserved words
		i32 lfanew;   // file address of new exe header
	};
}