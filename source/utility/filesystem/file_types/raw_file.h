#pragma once

#include "utility/filesystem/file_types/old_file.h"
#include <llvm/Support/raw_ostream.h>

namespace utility {
	class raw_file : public old_file {
	public:
		raw_file(
			const filepath& path,
			s_ptr<llvm::raw_fd_ostream> stream
		);

		static outcome::result<s_ptr<raw_file>> create(
			const filepath& path
		);

		s_ptr<llvm::raw_fd_ostream> get_stream() const;

		void write() const;
	private:
		s_ptr<llvm::raw_fd_ostream> m_stream;
	};
}
