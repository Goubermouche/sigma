#pragma once
#include "utility/macros.h"
#include "utility/containers/contiguous_container.h"

namespace utility {
	namespace types {
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
				try {
					return std::filesystem::canonical(m_path);
				}
				catch (const std::filesystem::filesystem_error& err) {
					SUPPRESS_C4100(err);
					PANIC("{}", err.what());
					return {};
				}
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
		private:
			path_type m_path;
		};
	}

	namespace fs {
		template<typename type>
		struct file {
			static auto save(const filepath& path, const type& value) -> result<void> {
				SUPPRESS_C4100(path);
				SUPPRESS_C4100(value);
				console::print("here 1\n");
				return SUCCESS;
			}

			static auto load(const filepath& path) -> result<type> {
				SUPPRESS_C4100(path);
				console::print("here 2\n");
				return SUCCESS;
			}
		};

		template<typename type>
		struct file<contiguous_container<type>> {
			static result<void> save(const filepath& path, const contiguous_container<type>& value) {
				std::ofstream file(path.get_path(), std::ios::binary);

				if (!file) {
					return error::create(error::code::CANNOT_READ_FILE, path);
				}

				for (const type* ptr = value.begin(); ptr != value.end(); ++ptr) {
					file.write(reinterpret_cast<const char*>(ptr), sizeof(type));
				}

				file.close();

				return SUCCESS;
			}
		};

		template<>
		struct file<std::string> {
			static auto load(const filepath& path) -> result<std::string> {
				std::ifstream file(path.get_path());

				if (!file) {
					return error::create(error::code::CANNOT_READ_FILE, path);
				}

				return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
			}
		};

		class directory {
		public:
			static void for_all_directory_items(const filepath& path, const std::function<void(const filepath&)>& function) {
				ASSERT(path.exists(), "directory '{}' doesn't exist", path);
				ASSERT(path.is_directory(), "filepath '{}' doesn't point to a directory", path);

				for (const auto& entry : std::filesystem::directory_iterator(path.get_path())) {
					function(entry.path());
				}
			}
		};

		inline auto remove(const filepath& path) -> bool {
			return std::filesystem::remove(path.get_path());
		}
	}
} // namespace utility::fs

template<>
struct std::formatter<utility::types::filepath> {
	template<typename parse_context>
	constexpr auto parse(parse_context& ctx) {
		return ctx.begin();
	}

	template<typename format_context>
	auto format(const utility::types::filepath& p, format_context& ctx) const {
		return format_to(ctx.out(), "{}", p.to_string());
	}
};
