#include "filesystem.h"

namespace language::detail {
	bool read_file(const std::string& source_file, std::string& out) {
		std::ifstream file(source_file);
		if (file.good()) {
			out = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			return true;
		}

		return false;
	}
}
