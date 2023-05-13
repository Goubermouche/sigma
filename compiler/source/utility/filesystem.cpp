#include "filesystem.h"

#include <llvm/Support/Path.h>

namespace channel::detail {
	bool read_file(
		const std::string& file_path,
		std::string& out
	) {
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

	std::string extract_directory_from_filepath(
		const std::string& filepath
	) {
		const std::filesystem::path path(filepath);

		if (is_directory(path)) {
			// if it's already a directory, just return it
			return path.string(); 
		}

		// if it's a file, return the parent directory
		return path.parent_path().string();
	}

	std::string extract_filename_from_filepath(
		const std::string& filepath
	) {
		const std::filesystem::path path(filepath);
		return path.stem().string();
	}

	bool is_file(
		const std::string& filepath
	) {
		return std::filesystem::is_regular_file(filepath);
	}

	bool is_directory(
		const std::string& filepath
	) {
		return std::filesystem::is_directory(filepath);
	}

	std::string extract_extension_from_filepath(
		const std::string& filepath
	) {
		const std::filesystem::path path(filepath);
		return path.extension().string();
	}
}
