#include "compilation_unit.h"

#include "llvm/Linker/Linker.h"
#include "llvm/Transforms/Utils/Cloning.h"

namespace sigma {
	compilation_unit::compilation_unit()
		: m_context(std::make_shared<code_generator_context>()) {}

	compilation_unit::compilation_unit(
		const token_list& token_list
	) : m_context(std::make_shared<code_generator_context>()) {
		m_parser.set_token_list(token_list);
	}

	outcome::result<void> compilation_unit::compile(
		const std::vector<std::shared_ptr<code_generator_context>>& dependencies
	) {
		// parse the given token list
		OUTCOME_TRY(m_parser.parse());

		// link all dependencies
		// todo: implement a linker class? 
		OUTCOME_TRY(link(dependencies));

		// generate llvm IR
		OUTCOME_TRY(m_code_generator.generate(m_context, m_parser.get_abstract_syntax_tree()));

		return outcome::success();
	}

	std::shared_ptr<code_generator_context> compilation_unit::get_context() const {
		return m_context;
	}
	outcome::result<void> compilation_unit::link(
		const std::vector<std::shared_ptr<code_generator_context>>& dependencies
	) const {
		if (!dependencies.empty()) {
			llvm::Linker linker(*m_context->get_module());
			// for (const auto& dependency : dependencies) {
			// 	dependency->print();
			// }

			for (const auto& dependency : dependencies) {
				if (linker.linkInModule(CloneModule(*dependency->get_module()))) {
					console::out
						<< color::red
						<< "linker error!"
						<< color::white;
				}

				OUTCOME_TRY(
					m_context->concatenate_function_registry(
						dependency->get_function_registry()
					)
				);

				OUTCOME_TRY(
					m_context->concatenate_variable_registry(
						dependency->get_variable_registry()
					)
				);
			}
		}

		return outcome::success();
	}
}
