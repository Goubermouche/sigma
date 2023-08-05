#pragma once
// file types
#include "utility/filesystem/file_types/text_file.h"
#include "utility/filesystem/file_types/raw_file.h"

namespace sigma {
	struct file_position {
		u64 line_index;
		u64 char_index;
	};

	struct file_range {
		std::shared_ptr<text_file> file;
		file_position start;
		file_position end;
	};

	class file_registry {
	public:
		void insert(
			const std::shared_ptr<file>& file,
			const filepath& path
		) {
			m_files[path] = file;
		}

		template<typename file_type>
		std::shared_ptr<file_type> get(
			const filepath& path
		) {
			return std::static_pointer_cast<file_type>(m_files[path]);
		}
	private:
		std::unordered_map<filepath, std::shared_ptr<file>> m_files;
	};
}