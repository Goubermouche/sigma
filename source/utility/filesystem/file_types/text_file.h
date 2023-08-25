#pragma once
#include "utility/filesystem/file_types/file.h"

namespace utility {
	class text_file : public file {
	public:
		text_file() = default;

		text_file(
			const filepath& path,
			const std::vector<std::string>& lines
		);

		const std::string& get_line(uint64_t index) const;
		const std::vector<std::string>& get_lines() const;

		 static outcome::result<ptr<text_file>> load(
		 	const filepath& path
		 );
	protected:
		std::vector<std::string> m_lines;
	};
}