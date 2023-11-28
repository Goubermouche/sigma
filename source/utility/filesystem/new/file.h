#pragma once
#include "utility/diagnostics/error.h"
#include "utility/containers/contiguous_container.h"

namespace utility {
	class file {
	public:
		template<typename type>
		static auto write(const contiguous_container<type>& data, const filepath& path) -> outcome::result<void> {
			std::ofstream file(path, std::ios::binary);

			if(!file) {
				return outcome::failure(error::emit_assembly<error_code::unable_to_open_file>(path));
			}

			for (const type* ptr = data.cbegin(); ptr != data.cend(); ++ptr) {
				file.write(reinterpret_cast<const char*>(ptr), sizeof(type));
			}

			file.close();
			return outcome::success();
		}
	};
}
