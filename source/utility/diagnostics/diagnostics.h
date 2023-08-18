#pragma once
#include "utility/macros.h"
#include "utility/format.h"
#include "utility/filesystem/filesystem.h"

namespace sigma {
	namespace detail {
		template<>
		struct formatter<filepath> {
			static std::string format(const filepath& value) {
				return value.string();
			}
		};
	}

	struct diagnostic_message {
		diagnostic_message(
			const std::string& message
		) : m_message(message) {}
		virtual void print() = 0;
	protected:
		std::string m_message;
	};
}