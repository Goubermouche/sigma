#pragma once
#include "utility/containers/contiguous_container.h"

namespace utility {
	class fs {
	public:
		static auto exists(const filepath& path) -> bool {
			return std::filesystem::exists(path);
		}

		static auto is_directory(const filepath& path) -> bool {
			return std::filesystem::is_directory(path);
		}

		static auto is_file(const filepath& path) -> bool {
			return std::filesystem::is_regular_file(path);
		}

		static auto get_canonical_path(const filepath& path) -> filepath {
			try {
				return canonical(path);
			}	catch(const std::filesystem::filesystem_error& err) {
				PANIC("{}", err.what());
				return {};
			}
		}

		static void remove(const filepath& path) {
			std::filesystem::remove(path);
		}
	};

	class file {
	public:
		template<typename type>
		static void write(const contiguous_container<type>& data, const filepath& path) {
			std::ofstream file(path, std::ios::binary);

			if(!file) {
				ASSERT(false, "fail");
				// return outcome::failure(error::emit_assembly<error_code::unable_to_open_file>(path));
			}

			for (const type* ptr = data.begin(); ptr != data.end(); ++ptr) {
				file.write(reinterpret_cast<const char*>(ptr), sizeof(type));
			}

			file.close();
		}

		static auto read_text_file(const filepath& path) -> result<std::string> {
			std::ifstream file(path);

			if(!file) {
				return error::create(error::code::CANNOT_READ_FILE, path.string());
			}

			return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
		}
	};

	class directory {
	public:
		static void for_all_directory_items(const filepath& path, std::function<void(const filepath&)> function) {
			ASSERT(fs::exists(path), "directory '{}' doesn't exist", path.string());
			ASSERT(fs::is_directory(path), "filepath '{}' doesn't point to a directory", path.string());

			for (const auto& entry : std::filesystem::directory_iterator(path)) {
				function(entry.path());
			}
		}
	};
}
