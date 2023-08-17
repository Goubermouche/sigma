#pragma once
// file types
#include "utility/filesystem/file_types/text_file.h"
#include "utility/filesystem/file_types/raw_file.h"

namespace sigma {
	struct file_position {
		u64 line_index;
		u64 char_index;
	};

	struct file_range {
		std::shared_ptr<text_file> file;
		file_position start;
		file_position end;
	};
}