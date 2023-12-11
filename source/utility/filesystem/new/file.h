#pragma once
#include "utility/containers/contiguous_container.h"

namespace utility {
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

		static auto read_text_file(const filepath& path) -> std::string {
			std::ifstream file(path);

			if(!file) {
				ASSERT(false, "failure");
			}

			return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
		}
	};
}
