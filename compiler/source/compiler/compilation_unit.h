#pragma once
#include "utility/diagnostics/error.h"

#include "code_generator/code_generator_context.h"

#include "parser/parser.h"
#include "code_generator/implementation/code_generator.h"

namespace sigma {
	class compilation_unit {
	public:
		compilation_unit();

		compilation_unit(
			const token_list& token_list
		);

		/**
		 * \brief Compiles the given compilation unit.
		 * \param dependencies Dependencies to use for compilation (ie. the included files)
		 * \return outcome (void)
		 */
		outcome::result<void> compile(
			const std::vector<std::shared_ptr<code_generator_context>>& dependencies
		);

		std::shared_ptr<code_generator_context> get_context() const;
	private:
		outcome::result<void> link(
			const std::vector<std::shared_ptr<code_generator_context>>& dependencies
		) const;
	private:
		parser m_parser;
		code_generator m_code_generator;

		std::shared_ptr<code_generator_context> m_context;
	};
}
