#include "text_file.h"
#include "utility/diagnostics/error.h"

namespace utility {
	text_file::text_file(const filepath& path, std::fstream stream)
		: old_file(path), m_stream(std::move(stream)) {}

	text_file::~text_file() {
		m_stream.close();
	}

	outcome::result<s_ptr<text_file>> text_file::load(const filepath& path) {
		std::fstream stream;
		stream.open(path, std::ios::in | std::ios::out | std::ios::app);

		if(!stream.is_open()) {
			return outcome::failure(error::emit_assembly<error_code::unable_to_open_file>(
				path
			));
		}

		return std::make_shared<text_file>(path, std::move(stream));
	}

	outcome::result<s_ptr<text_file>> text_file::create(
		const filepath& path
	) {
		std::fstream stream;
		stream.open(path, std::fstream::out);

		if (!stream.is_open()) {
			return outcome::failure(error::emit_assembly<error_code::unable_to_open_file>(
				path
			));
		}

		return std::make_shared<text_file>(path, std::move(stream));
	}

	std::string text_file::get_contents() const {
		m_stream.seekg(0, std::ios::end);
		std::streamsize size = m_stream.tellg();
		m_stream.seekg(0, std::ios::beg);

		std::string content(size, '\0');
		if (!m_stream.read(&content[0], size)) {
			ASSERT(false, "cannot read file, unhandled error");
		}

		m_stream.clear();  // Clear EOF flag
		return content;
	}

	std::string text_file::get_line(u64 index) const {
		m_stream.seekg(0, std::ios::beg);
		std::string line;
		line.reserve(100);  // reserve some space to reduce reallocations

		for (u64 i = 0; std::getline(m_stream, line); ++i) {
			if (i == index) {
				return line;
			}
		}

		ASSERT(false, "index out of range");
		return std::string();
	}

	s_ptr<text_file> operator<<(s_ptr<text_file> file, const std::string& value) {
		file->m_stream << value;
		return file;
	}

	s_ptr<text_file> operator<<(s_ptr<text_file> file, u64 value) {
		file->m_stream << std::to_string(value);
		return file;
	}

	s_ptr<text_file> operator<<(s_ptr<text_file> file, i64 value) {
		file->m_stream << std::to_string(value);
		return file;
	}

	s_ptr<text_file> operator<<(s_ptr<text_file> file, i32 value) {
		file->m_stream << std::to_string(value);
		return file;
	}

	s_ptr<text_file> operator<<(s_ptr<text_file> file, i16 value) {
		file->m_stream << std::to_string(value);
		return file;
	}

	s_ptr<text_file> operator<<(s_ptr<text_file> file, i8 value) {
		file->m_stream << std::to_string(value);
		return file;
	}
}