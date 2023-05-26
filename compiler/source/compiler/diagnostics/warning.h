#pragma once
#include "compiler/diagnostics/diagnostic.h"

namespace channel {
	constexpr auto warning_templates = make_hash_map(
		// codegen
		std::pair{ 3000, "'{}': implicit function return generated" },
		std::pair{ 3001, "'initializing': implicit function type cast '{}' to '{}'" },
		std::pair{ 3002, "'initializing': implicit type cast '{}' to '{}'" }
	);

	class warning_message : public diagnostic_message {
	public:
		warning_message(std::string message, u64 code);

		void print() const override;
	};

	class warning_message_position : public warning_message {
	public:
		warning_message_position(std::string message, u64 code, token_position position);

		void print() const override;
	protected:
		token_position m_position;
	};

	class warning {
	public:
		template <u64 code, typename... Args>
		static warning_message emit(Args&&... args);

		template <u64 code, typename... Args>
		static warning_message_position emit(token_position position, Args&&... args);
	};

	template<u64 code, typename ...Args>
	warning_message warning::emit(Args && ...args) {
		return {
			std::format(warning_templates[code], std::forward<Args>(args)...),
			code
		};
	}

	template<u64 code, typename ...Args>
	warning_message_position warning::emit(token_position position, Args && ...args) {
		return {
			std::format(warning_templates[code], std::forward<Args>(args)...),
			code,
			position
		};
	}
}