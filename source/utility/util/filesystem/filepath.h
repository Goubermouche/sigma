#pragma once
#include "../types.h"

namespace utility::types {
	class filepath {
	public:
		using path_type = std::filesystem::path;

		filepath() = default;
		filepath(const path_type& path) : m_path(path) {}
		filepath(const std::string& path) : m_path(path) {}
		filepath(const char* path) : m_path(path) {}

		auto operator==(const filepath& other) const -> bool {
			return m_path == other.m_path;
		}

		auto operator/(const filepath& other) const -> filepath {
			return m_path / other.m_path;
		}

		auto exists() const -> bool {
			return std::filesystem::exists(m_path);
		}

		auto is_directory() const -> bool {
			return std::filesystem::is_directory(m_path);
		}

		auto is_file() const -> bool {
			return std::filesystem::is_regular_file(m_path);
		}

		auto get_canonical_path() const -> filepath {
			return std::filesystem::canonical(m_path);
		}

		auto to_string() const -> std::string {
			return m_path.string();
		}

		auto remove() const -> bool {
			return std::filesystem::remove(m_path);
		}

		auto get_parent_path() const -> filepath {
			return m_path.parent_path();
		}

		auto get_path() const -> const path_type& {
			return m_path;
		}

		auto get_extension() const -> filepath {
			return m_path.extension();
		}

		auto get_filename() const -> filepath {
			return m_path.filename();
		}

		auto get_filename_no_ext() const -> filepath {
			return m_path.stem();
		}

		auto operator/(const filepath& other) {
			return m_path / other.m_path;
		}
	private:
		path_type m_path;
	};
} // namespace utility::types

template<>
struct std::formatter<utility::types::filepath> {
	template<typename parse_context>
	constexpr auto parse(parse_context& ctx) const {
		return ctx.begin();
	}

	template<typename format_context>
	auto format(const utility::types::filepath& p, format_context& ctx) const {
		return format_to(ctx.out(), "{}", p.to_string());
	}
};
