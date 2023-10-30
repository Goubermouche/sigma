#include "old_file.h"
#include "utility/diagnostics/error.h"

namespace utility {
	old_file::old_file(const filepath& path)
		: m_path(path) {}

	const filepath& old_file::get_path() const {
		return m_path;
	}

	std::string old_file::get_extension() const {
		return m_path.extension().string();
	}

	outcome::result<void> old_file::verify(const filepath& path) {
		if (is_directory(path)) {
			return outcome::failure(
				error::emit_assembly<error_code::file_expected_but_got_directory>(path)
			);
		}

		return outcome::success();
	}

	outcome::result<void> old_file::remove(
		const filepath& path
	) {
		if (std::remove(path.string().c_str())) {
			return outcome::failure(
				error::emit_assembly<error_code::unable_to_delete_file>(path)
			);
		}

		return outcome::success();
	}
}
