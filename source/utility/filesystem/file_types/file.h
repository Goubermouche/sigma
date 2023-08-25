#pragma once
#include "utility/macros.h"

namespace utility {
	namespace outcome {
		template<typename type>
		class result;
	}

	class file {
	public:
		file() = default;

		file(
			const filepath& path
		);

		static outcome::result<void> remove(
			const filepath& path
		);

		const filepath& get_path() const;
		std::string get_extension() const;
	protected:
		static outcome::result<void> verify(
			const filepath& path
		);
	protected:
		filepath m_path;

		friend class file_registry;
	};
}
