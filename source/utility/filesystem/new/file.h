#pragma once
#include "utility/diagnostics/error.h"


namespace utility {
	class file {
	public:
		/*static outcome::result<void> write(const long_string& value, const filepath& path) {
			OUTCOME_TRY(std::ofstream& file, get_write_handle(path));

			for (char* chunk : value) {
				file.write(chunk, std::strlen(chunk));
			}

			return outcome::success();
		}*/

		//static outcome::result<void> write(const byte_buffer& value, const filepath& path) {
		//	OUTCOME_TRY(std::ofstream& file, get_write_handle(path));

		//	file.write(reinterpret_cast<const char*>(
		//		value.get_data()), value.get_size() * sizeof(utility::byte)
		//	);

		//	return outcome::success();
		//}
	private:
		static outcome::result<std::ofstream> get_write_handle(
			const filepath& path
		) {
			std::ofstream stream(path, std::ios::binary);

			if (!stream.is_open()) {
				stream.close();

				return outcome::failure(
					error::emit_assembly<error_code::unable_to_open_file>(path)
				);
			}

			return stream;
		}
	};
}
