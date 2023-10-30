#pragma once
#include "utility/containers/handle.h"
#include "utility/filesystem/file_types/old_file.h"

namespace utility {
	class text_file : public old_file {
	public:
		text_file() = default;

		text_file(
			const filepath& path,
			std::fstream stream
		);

		~text_file();

		static outcome::result<s_ptr<text_file>> load(
		 	const filepath& path
		);

		static outcome::result<s_ptr<text_file>> create(
			const filepath& path
		);

		std::string get_contents() const;
		std::string get_line(u64 index) const;

		friend s_ptr<text_file> operator<<(
			s_ptr<text_file> file,
			const std::string& value
		);

		friend s_ptr<text_file> operator<<(
			s_ptr<text_file> file,
			u64 value
		);

		friend s_ptr<text_file> operator<<(
			s_ptr<text_file> file,
			i64 value
		);

		friend s_ptr<text_file> operator<<(
			s_ptr<text_file> file,
			i32 value
		);

		friend s_ptr<text_file> operator<<(
			s_ptr<text_file> file,
			i16 value
		);

		friend s_ptr<text_file> operator<<(
			s_ptr<text_file> file,
			i8 value
		);

		template<typename type>
		friend s_ptr<text_file> operator<<(
			s_ptr<text_file> file,
			handle<type> value
		);
	protected:
		friend class console;

		mutable std::fstream m_stream;
	};

	template<typename type>
	s_ptr<text_file> operator<<(s_ptr<text_file> file, handle<type> value) {
		file->m_stream << std::to_string(reinterpret_cast<uintptr_t>(value.get()));
		return file;
	}
}