#include "text_file.h"
#include "utility/diagnostics/error.h"

namespace sigma {
	text_file::text_file(
		const filepath& path, 
		const std::vector<std::string>& lines
	) : file(path),
	m_lines(lines) {}

	const std::string& text_file::get_line(uint64_t index) const {
		return m_lines[index];
	}

	const std::vector<std::string>& text_file::get_lines() const {
		return m_lines;
	}

	 outcome::result<std::shared_ptr<text_file>> text_file::load(
	 	const filepath& path
	 ) {
		 OUTCOME_TRY(verify(path));
	 	 std::ifstream file(path);
	 
	 	 if (file.is_open()) {
	 		 std::vector<std::string> lines;
	 		 std::string line;
	 
	 		 while (std::getline(file, line)) {
	 			 lines.push_back(line);
	 		 }
	 		 file.close();
	 
	 	 	 return std::make_shared<text_file>(path, lines);
	 	 }
	 
	 	 return outcome::failure(
	 		 error::emit<error_code::unable_to_open_file>(path)
	 	 );
	 }
}