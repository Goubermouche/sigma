#pragma once
#include "utility/diagnostics/diagnostic.h"

namespace sigma {
	namespace diagnostics {}
	constexpr auto warning_templates = make_hash_map(
		// *********************************************************************************************************************
		// codegen warnings
		// *********************************************************************************************************************
		std::pair{ 3000, "'{}': implicit function return generated" },
		std::pair{ 3001, "'initializing': implicit function type cast '{}' to '{}'" },
		std::pair{ 3002, "'initializing': implicit type cast '{}' to '{}'" }
	);

	class warning_message : public diagnostic_message {
	public:
		warning_message(std::string message, u64 code);

		void print() const override;
	};

	using warning_msg = std::shared_ptr<warning_message>;

	class warning_message_position : public warning_message {
	public:
		warning_message_position(
			std::string message,
			u64 code, 
			const file_position& position
		);

		void print() const override;
	protected:
		file_position m_position;
	};

	class warning {
	public:
		template <u64 code, typename... Args>
		static warning_msg emit(
			Args&&... args
		);

		template <u64 code, typename... Args>
		static std::shared_ptr<warning_message_position> emit(
			file_position position,
			Args&&... args
		);
	};

	template<u64 code, typename ...Args>
	warning_msg warning::emit(
		Args && ...args
	) {
		return std::make_shared<warning_message>(
			std::format(warning_templates[code],
			std::forward<Args>(args)...),
			code
		);
	}

	template<u64 code, typename ...Args>
	std::shared_ptr<warning_message_position> warning::emit(
		file_position position,
		Args && ...args
	) {
		return std::make_shared<warning_message_position>(
			std::format(warning_templates[code],
			std::forward<Args>(args)...),
			code,
			position
		);
	}
}