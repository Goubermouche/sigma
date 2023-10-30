#include "console.h"
#include "utility/macros.h"
#include "utility/filesystem/file_types/text_file.h"

namespace utility {
	console& console::out = *new console();

	void console::set_output_stream(std::ostream& stream) {
		out.m_stream = &stream;
	}

	void console::set_output_stream(s_ptr<text_file> file) {
		out.m_stream = &file->m_stream;
	}

	console& console::width(i64 width) {
		*out.m_stream << std::setw(width);
		return out;
	}

	console& console::precision(i64 precision) {
		*out.m_stream << std::fixed << std::setprecision(precision);
		return out;
	}

	console& console::operator<<(const color_value& color) {
		*m_stream << color.ansi_code;
		return *this;
	}

	console& console::operator<<(const left_pad& left_pad) {
		SUPPRESS_C4100(left_pad);
		*m_stream << std::left;
		return *this;
	}

	console& console::operator<<(const right_pad& right_pad) {
		SUPPRESS_C4100(right_pad);
		*m_stream << std::right;
		return *this;
	}

	console& console::operator<<(const console& console) {
		SUPPRESS_C4100(console);
		return *this;
	}

	console& console::operator<<(const std::string& value) {
		// const std::wstring wide_string(value.begin(), value.end());
		*m_stream << value;
		return *this;
	}

	console& console::operator<<(const filepath& value) {
		*m_stream << value.string();
		return *this;
	}

	console& console::operator<<(const char* value) {
		*m_stream << value;
		return *this;
	}

	console& console::operator<<(char value) {
		*m_stream << value;
		return *this;
	}

	console& console::operator<<(bool value) {
		if(value) {
			*m_stream << "true";
		}
		else {
			*m_stream << "false";
		}

		return *this;
	}

	console& console::operator<<(f32 value) {
		*m_stream << value;
		return *this;
	}

	console& console::operator<<(f64 value) {
		*m_stream << value;
		return *this;
	}

	console& console::operator<<(u8 value) {
		*m_stream << value;
		return *this;
	}

	console& console::operator<<(u16 value) {
		*m_stream << value;
		return *this;
	}

	console& console::operator<<(u32 value) {
		*m_stream << value;
		return *this;
	}

	console& console::operator<<(u64 value) {
		*m_stream << value;
		return *this;
	}

	console& console::operator<<(i8 value) {
		*m_stream << value;
		return *this;
	}

	console& console::operator<<(i16 value) {
		*m_stream << value;
		return *this;
	}

	console& console::operator<<(i32 value) {
		*m_stream << value;
		return *this;
	}

	console& console::operator<<(i64 value) {
		*m_stream << value;
		return *this;
	}
}
