#include "filesystem.h"

namespace sigma::detail {
	outcome::result<std::string> read_file(
		const filepath& path
	) {
		std::ifstream file(path, std::ios::in | std::ios::binary);

		if (!file.is_open()) {
			return outcome::failure(
				error::emit<1000>(path)
			);
		}

		std::string contents;
		file.seekg(0, std::ios::end);

		if (file.fail()) {
			// clear the error state
			file.clear();
			file.close();

			return outcome::failure(
				error::emit<1005>(path)
			);
		}

		contents.resize(file.tellg());
		file.seekg(0, std::ios::beg);
		file.read(&contents[0], contents.size());

		if (file.fail()) {
			// clear the error state
			file.clear();
			file.close();

			return outcome::failure(
				error::emit<1006>(path)
			);
		}

		file.close();

		return contents;
	}

	bool delete_file(const filepath& path) {
		return std::remove(path.string().c_str()) == 0;
	}

	std::string extract_directory_from_filepath(
		const filepath& path
	) {
		if (std::filesystem::is_directory(path)) {
			// if it's already a directory, just return it
			return path.string(); 
		}

		// if it's a file, return the parent directory
		return path.parent_path().string();
	}

	std::string extract_filename_from_filepath(
		const filepath& path
	) {
		return path.stem().string();
	}

	bool is_file(
		const filepath& path
	) {
		return std::filesystem::is_regular_file(path);
	}

	bool is_directory(
		const filepath& path
	) {
		return std::filesystem::is_directory(path);
	}

	std::string extract_extension_from_filepath(
		const filepath& path
	) {
		return path.extension().string();
	}
}
