#include "compilation_context.h"

namespace sigma {
	backend_context::backend_context(sigma::syntax& syntax, ir::target target)
		: allocator(1024), syntax(syntax), semantics(*this), module(target), builder(module) {
		// TODO: we don't have to initialize these if they're not used

		// printf
		{
			const utility::string_table_key printf_key = this->syntax.strings.insert("printf");
			const utility::string_table_key format_key = this->syntax.strings.insert("format");

			auto printf_params = utility::slice<named_data_type>(this->syntax.ast.get_allocator(), 1);
			printf_params[0] = named_data_type{ type::create_char(1), format_key };

			const function_signature printf_function = {
				.return_type = type::create_i32(),
				.parameter_types = printf_params,
				.has_var_args = true,
				.identifier_key = printf_key
			};

			semantics.declare_external_function(printf_function);
		}

		// malloc
		{
			const utility::string_table_key malloc_key = this->syntax.strings.insert("malloc");
			const utility::string_table_key size_key = this->syntax.strings.insert("size");
		
			auto malloc_params = utility::slice<named_data_type>(this->syntax.ast.get_allocator(), 1);
			malloc_params[0] = named_data_type{ type::create_u64(), size_key };
		
			const function_signature malloc_function = {
				.return_type = type::create_void(1),
				.parameter_types = malloc_params,
				.has_var_args = false,
				.identifier_key = malloc_key
			};
		
			semantics.declare_external_function(malloc_function);
		}
	}

	frontend_context::frontend_context()
		: allocator(sizeof(token_location) * 200) {}

	void frontend_context::print_tokens() const {
		if (tokens.empty()) {
			utility::console::print("empty token buffer\n");
			return;
		}

		// find the longest line x char index combination so that we can align our tokens for easier viewing
		u8 longest_line_number = 1;
		u8 longest_char_number = 1;

		for (const auto& info : tokens) {
			longest_line_number = std::max(longest_line_number, utility::num_digits(info.location->line_index + 1));
			longest_char_number = std::max(longest_char_number, utility::num_digits(info.location->char_index + 1));
		}

		u8 longest_loc = longest_line_number + longest_char_number + 2; // + 1 because we add a ':' char as well

		for (const auto& info : tokens) {
			std::string symbol_value;
			// check if the token has a string value associated with it 
			if (syntax.strings.contains(info.symbol_key)) {
				symbol_value = utility::escape_string(syntax.strings.get(info.symbol_key));
			}

			utility::console::print(
				"{:<{}} {:<20} {}\n",
				std::format("{}:{}:", info.location->line_index + 1, info.location->char_index + 1),
				longest_loc,
				info.tok.to_string(),
				symbol_value
			);
		}
	}

	void syntax::print_ast() const {
		ast.traverse([&](const handle<ast::node>& node, u16 depth) {
			utility::console::print("{}{} ", std::string(static_cast<u64>(depth * 2), ' '), node->type.to_string());

			switch (node->type) {
				// declarations
				case ast::node_type::FUNCTION_DECLARATION: {
					const auto& property = node->get<ast::function>();
					utility::console::print("['{} {} (", property.signature.return_type.to_string(), strings.get(property.signature.identifier_key));

					for (u64 i = 0; i < property.signature.parameter_types.get_size(); ++i) {
						utility::console::print("{}", property.signature.parameter_types[i].type.to_string());

						if (i + 1 != property.signature.parameter_types.get_size()) {
							utility::console::print(", ");
						}
					}
					utility::console::print(")']");
					break;
				}
				case ast::node_type::NAMESPACE_DECLARATION: {
					const auto& property = node->get<ast::named_expression>();
					utility::console::print("['{}']", strings.get(property.key));
					break;
				}
				case ast::node_type::VARIABLE_DECLARATION: {
					const auto& property = node->get<ast::named_type_expression>();
					utility::console::print("[{} '{}']", strings.get(property.key), property.type.to_string());
					break;
				}
				case ast::node_type::STRUCT_DECLARATION: {
					const auto& property = node->get<ast::named_type_expression>();
					const auto& members = property.type.get_struct_members();

					utility::console::print("['{} {{", strings.get(property.key));

					for(u8 i = 0; i < members.get_size(); ++i) {
						utility::console::print(members[i].to_string());

						if(i + 1 != members.get_size()) {
							utility::console::print(", ");
						}
					}

					utility::console::print("}}'");
					break;
				}

				case ast::node_type::FUNCTION_CALL: {
					const auto& property = node->get<ast::function_call>();

					utility::console::print("['");

					for (const utility::string_table_key key : property.namespaces) {
						utility::console::print("{}::", strings.get(key));
					}

					utility::console::print("{}']", strings.get(property.signature.identifier_key));
					break;
				}
				case ast::node_type::VARIABLE_ACCESS: {
					const auto& property = node->get<ast::named_type_expression>();
					utility::console::print("[{}]", strings.get(property.key));
					break;
				}

				// literals
				case ast::node_type::NUMERICAL_LITERAL: {
					const auto& property = node->get<ast::named_type_expression>();
					utility::console::print("['{}' {}]", property.type.to_string(), strings.get(property.key));
					break;
				}
				case ast::node_type::STRING_LITERAL: {
					const auto& property = node->get<ast::named_type_expression>();
					utility::console::print("[\"{}\"]", utility::escape_string(strings.get(property.key)));
					break;
				}
				case ast::node_type::BOOL_LITERAL: {
					const auto& property = node->get<ast::bool_literal>();
					utility::console::print("[{}]", property.value ? "true" : "false");
					break;
				}

				default: break; // suppress unhandled enumeration warnings
			}

			utility::console::print("\n");
		});
	}
} // namespace sigma

