#pragma once

#include "utility/filesystem/file_types/file.h"
#include <llvm/Support/raw_ostream.h>

namespace utility {
	class raw_file : public file {
	public:
		raw_file(
			const filepath& path,
			ptr<llvm::raw_fd_ostream> stream
		);

		static outcome::result<ptr<raw_file>> create(
			const filepath& path
		);

		ptr<llvm::raw_fd_ostream> get_stream() const;

		void write() const;
	private:
		ptr<llvm::raw_fd_ostream> m_stream;
	};
}
