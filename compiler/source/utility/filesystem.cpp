#include "filesystem.h"

#include <llvm/Support/Path.h>

namespace channel::detail {
	bool read_file(const std::string& file_path, std::string& out) {
		std::ifstream file(file_path);

		if (file.good()) {
			// note: keep the additional parentheses around the first
			//       argument for std::string, this is required if we
			//       want to compile everything successfully.
			out = std::string((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
			return true;
		}

		return false;
	}

	bool delete_file(const std::string& file_path) {
		return std::remove(file_path.c_str()) == 0;
	}
}
