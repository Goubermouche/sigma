#pragma once
#include "utility/macros.h"
#include "llvm_wrappers/type.h"
#include "utility/containers/compile_time_hash_map.h"

template<>
struct std::formatter<channel::type> {
	static auto parse(format_parse_context& ctx) {
		return ctx.begin();
	}

	static auto format(channel::type e, format_context& ctx) {
		return format_to(ctx.out(), "{}", e.to_string());
	}
};

template<>
struct std::formatter<channel::token> {
	static auto parse(format_parse_context& ctx) {
		return ctx.begin();
	}

	static auto format(channel::token e, format_context& ctx) {
		return format_to(ctx.out(), "{}", channel::token_to_string(e));
	}
};

namespace channel {
	class diagnostic_message {
	public:
		virtual void print() const = 0;

		diagnostic_message(std::string message, u64 code);
	protected:
		std::string m_message;
		u64 m_code;
	};
}