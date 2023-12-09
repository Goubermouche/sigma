#pragma once
// file types
#include "utility/filesystem/file_types/text_file.h"

namespace utility {
	struct file_position {
		u64 line_index;
		u64 char_index;
	};

	struct file_range {
		s_ptr<text_file> file;
		file_position start;
		file_position end;
	};
}
