#include "compilation_context.h"
#include <utility/string_helper.h>

namespace sigma {
	void compilation_context::print_tokens() const {
		for(const auto& info : tokens) {
			std::string symbol_value;
			if(strings.contains(info.symbol_key)) {
				symbol_value = utility::detail::escape_string(strings.get(info.symbol_key));
			}

			utility::console::println("{:<20} {}", info.tok.to_string(), symbol_value);
		}
	}

	void compilation_context::print_ast() const {
		ast.traverse([&](const handle<node>& node, u16 depth) {
			utility::console::print(
				"{}{} ", std::string(static_cast<u64>(depth * 2), ' '), node->type.to_string()
			);

			switch (node->type) {
				case node_type::FUNCTION_DECLARATION: {
					const auto& property = node->get<function>();

					utility::console::print(
						"['{} {} (", property.return_type.to_string(), strings.get(property.identifier_key)
					);

					for (u64 i = 0; i < property.parameter_types.get_size(); ++i) {
						utility::console::print("{}", property.parameter_types[i].type.to_string());

						if(i + 1 != property.parameter_types.get_size()) {
							utility::console::print(", ");
						}
					}
					utility::console::print(")']");
					break;
				}

				case node_type::VARIABLE_DECLARATION: {
					const auto& property = node->get<variable>();

					utility::console::print(
						"[{} '{}']", strings.get(property.identifier_key), property.dt.to_string()
					);
					break;
				}
				case node_type::VARIABLE_ACCESS: {
					const auto& property = node->get<variable>();
					utility::console::print("[{}]", strings.get(property.identifier_key));
					break;
				}

				case node_type::NUMERICAL_LITERAL: {
					const auto& property = node->get<literal>();
					utility::console::print(
						"['{}' {}]", property.dt.to_string(), strings.get(property.value_key)
					);

					break;
				}
				case node_type::STRING_LITERAL: {
					const auto& property = node->get<literal>();
					utility::console::print(
						"[\"{}\"]", utility::detail::escape_string(strings.get(property.value_key))
					);

					break;
				}
				case node_type::BOOL_LITERAL: {
					const auto& property = node->get<bool_literal>();
					utility::console::print(
						"[{}]", property.value ? "true" : "false"
					);

					break;
				}

				default: break; // suppress unhandled enumeration warnings
			}

			utility::console::println();
		});
	}
} // namespace sigma

