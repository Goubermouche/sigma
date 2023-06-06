#include "filesystem.h"

namespace channel::detail {
	std::expected<std::string, error_msg> read_file(
		const std::string& filepath
	) {
		std::ifstream file(filepath, std::ios::in | std::ios::binary);

		if (!file.is_open()) {
			return std::unexpected(
				error::emit<1000>(filepath)
			);
		}

		std::string contents;
		file.seekg(0, std::ios::end);

		if (file.fail()) {
			// clear the error state
			file.clear();
			file.close();

			return std::unexpected(
				error::emit<1005>(filepath)
			);
		}

		contents.resize(file.tellg());
		file.seekg(0, std::ios::beg);
		file.read(&contents[0], contents.size());

		if (file.fail()) {
			// clear the error state
			file.clear();
			file.close();

			return std::unexpected(
				error::emit<1006>(filepath)
			);
		}

		file.close();

		return { (contents) };
	}

	bool delete_file(const std::string& filepath) {
		return std::remove(filepath.c_str()) == 0;
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
