#include "compilation_context.h"
#include <utility/string_helper.h>

namespace sigma {
	void syntax::print_ast() const {
		ast.traverse([&](const handle<node>& node, u16 depth) {
			utility::console::print("{}{} ", std::string(static_cast<u64>(depth * 2), ' '), node->type.to_string());

			switch (node->type) {
				case node_type::FUNCTION_DECLARATION: {
					const auto& property = node->get<function_signature>();
					utility::console::print("['{} {} (", property.return_type.to_string(), string_table.get(property.identifier_key));

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
					utility::console::print("[{} '{}']", string_table.get(property.identifier_key), property.type.to_string());
					break;
				}
				case node_type::VARIABLE_ACCESS: {
					const auto& property = node->get<variable>();
					utility::console::print("[{}]", string_table.get(property.identifier_key));
					break;
				}

				case node_type::NUMERICAL_LITERAL: {
					const auto& property = node->get<literal>();
					utility::console::print("['{}' {}]", property.type.to_string(), string_table.get(property.value_key));
					break;
				}
				case node_type::STRING_LITERAL: {
					const auto& property = node->get<literal>();
					utility::console::print("[\"{}\"]", utility::detail::escape_string(string_table.get(property.value_key)));
					break;
				}
				case node_type::BOOL_LITERAL: {
					const auto& property = node->get<bool_literal>();
					utility::console::print("[{}]", property.value ? "true" : "false");
					break;
				}

				default: break; // suppress unhandled enumeration warnings
			}

			utility::console::print("\n");
		});
	}

	void frontend_context::print_tokens() const {
		for (const auto& info : tokens) {
			std::string symbol_value;
			if (syntax.string_table.contains(info.symbol_key)) {
				symbol_value = utility::detail::escape_string(syntax.string_table.get(info.symbol_key));
			}

			utility::console::print("{:<20} {}\n", info.tok.to_string(), symbol_value);
		}
	}

  backend_context::backend_context(sigma::syntax syntax, ir::target target)
	  : syntax(std::move(syntax)), module(target), builder(module), function_registry(*this), variable_registry(*this) {

		const utility::string_table_key printf_key = this->syntax.string_table.insert("printf");
		const utility::string_table_key format_key = this->syntax.string_table.insert("format");

		auto printf_params = utility::slice<named_data_type>(this->syntax.ast.get_allocator(), 1);
		printf_params[0] = named_data_type{ data_type(data_type::CHAR, 1), format_key };

		const function_signature printf_function = {
			.return_type = data_type(data_type::I32, 0),
			.parameter_types = printf_params,
			.has_var_args = true,
			.identifier_key = printf_key
		};

		function_registry.declare_external_function(printf_function);
	}
} // namespace sigma

