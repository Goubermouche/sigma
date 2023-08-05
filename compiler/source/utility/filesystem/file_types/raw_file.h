#pragma once

#include "utility/filesystem/file_types/file.h"
#include "llvm/Support/raw_ostream.h"

namespace sigma {
	class raw_file : public file {
	public:
		raw_file(
			const filepath& path,
			std::shared_ptr<llvm::raw_fd_ostream> stream
		);

		static outcome::result<std::shared_ptr<raw_file>> create(
			const filepath& path
		);

		std::shared_ptr<llvm::raw_fd_ostream> get_stream() const;

		void write() const;
	private:
		std::shared_ptr<llvm::raw_fd_ostream> m_stream;
	};
}
