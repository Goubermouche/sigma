#pragma once
#include "diagnostics/diagnostics.h"

namespace sigma {
	enum class warning_code {
		// ******************************************************************
		// codegen
		// ******************************************************************
		implicit_function_return_generated = 4000,
		implicit_function_type_cast = 4001,
		implicit_type_cast = 4002
	};

	struct warning_message : public diagnostic_message {
		warning_message(
			warning_code code,
			const std::string& message
		);

		virtual void print() override;
	protected:
		warning_code m_code;
	};

	struct warning_message_range : public warning_message {
		warning_message_range(
			warning_code code,
			const std::string& message,
			const file_range& range
		);

		void print() override;
	protected:
		file_range m_range;
	};

	class warning {
	public:
		template<warning_code code, typename...argument_types>
		static std::shared_ptr<warning_message> emit(argument_types... args) {
			const auto it = m_error_templates.find(code);
			if (it == m_error_templates.end()) {}

			return std::make_shared<warning_message>(code, detail::format(it->second, args...));
		}

		template<warning_code code, typename...argument_types>
		static std::shared_ptr<warning_message_range> emit(
			const file_range& range,
			argument_types... args
		) {
			const auto it = m_error_templates.find(code);
			if (it == m_error_templates.end()) {}

			return std::make_shared<warning_message_range>(code, detail::format(it->second, args...), range);
		}
	private:
		static inline std::unordered_map<warning_code, std::string> m_error_templates = {
			{ warning_code::implicit_function_return_generated, "'{}': implicit function return generated"                 },
			{ warning_code::implicit_function_type_cast,        "'initializing': implicit function type cast '{}' to '{}'" },
			{ warning_code::implicit_type_cast,                 "'initializing': implicit type cast '{}' to '{}'"          }
		};
	};
}