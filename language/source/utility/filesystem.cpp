#include "filesystem.h"

namespace language::detail {
	bool read_file(const std::string& source_file, std::string& out) {
		std::ifstream file(source_file);

		if (file.good()) {
			// note: keep the additional parentheses around the first
			//       argument for std::string, this is required if we
			//       want to compile everything successfully.
			out = std::string((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
			return true;
		}

		return false;
	}
}
