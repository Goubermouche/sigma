#include "file.h"
#include "utility/diagnostics/error.h"

namespace sigma {
	file::file(const filepath& path)
		: m_path(path) {}

	const filepath& file::get_path() const {
		return m_path;
	}

	std::string file::get_extension() const {
		return m_path.extension().string();
	}

	outcome::result<void> file::verify(const filepath& path) {
		if (is_directory(path)) {
			return outcome::failure(
				error::emit<error_code::file_expected_but_got_directory>(path)
			);
		}

		return outcome::success();
	}

	outcome::result<void> file::remove(
		const filepath& path
	) {
		if (std::remove(path.string().c_str())) {
			return outcome::failure(
				error::emit<error_code::unable_to_delete_file>(path)
			);
		}

		return outcome::success();
	}
}
