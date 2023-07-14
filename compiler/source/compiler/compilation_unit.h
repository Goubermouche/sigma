#pragma once
#include "code_generator/code_generator_context.h"
#include "utility/diagnostics/error.h"
#include "parser/parser.h"

namespace sigma {
	class compilation_unit {
	public:
		compilation_unit() = default;
		compilation_unit(
			const token_list& token_list
		);

		outcome::result<void> compile(
			const std::vector<std::shared_ptr<code_generator_context>>& dependencies
		);

		std::shared_ptr<code_generator_context> get_context() const;
	private:
		parser m_parser;
		std::shared_ptr<code_generator_context> m_context;
	};
}
