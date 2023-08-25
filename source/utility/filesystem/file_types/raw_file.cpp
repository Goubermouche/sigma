#include "raw_file.h"
#include "utility/diagnostics/error.h"
#include <llvm/Support/VirtualFileSystem.h>

namespace utility {
	raw_file::raw_file(
		const filepath& path, 
		ptr<llvm::raw_fd_ostream> stream
	) : file(path), m_stream(stream) {}

	outcome::result<ptr<raw_file>> raw_file::create(
		const filepath& path
	) {
		OUTCOME_TRY(verify(path));

		std::error_code error_code;
		const auto stream = std::make_shared<llvm::raw_fd_ostream>(
			path.string(),
			error_code,
			llvm::sys::fs::OF_None
		);

		// check for errors 
		if(error_code) {
			return outcome::failure(
				error::emit<error_code::unable_to_open_file_reason>(
					path,
					error_code.message()
				)
			);
		}

		return std::make_shared<raw_file>(
			path,
			stream
		);
	}

	ptr<llvm::raw_fd_ostream> raw_file::get_stream() const {
		return m_stream;
	}

	void raw_file::write() const {
		m_stream->flush();
	}
}