#pragma once
#include "parser/data_type.h"

#include <tokenizer/token_buffer.h>
#include <abstract_syntax_tree/abstract_syntax_tree.h>

#include "compiler/compiler/compilation_context.h"

namespace sigma {
	using namespace utility::types;

	struct frontend_context;

	class
	parser {
	public:
		[[nodiscard]] static auto parse(frontend_context& context) -> utility::result<void>;
	private:
		parser(frontend_context& context);
		[[nodiscard]] auto parse() -> utility::result<void>;

		auto parse_function_declaration() -> utility::result<handle<node>>;
		auto parse_namespace_declaration() -> utility::result<handle<node>>;

		auto parse_namespace_block() -> utility::result<std::vector<handle<node>>>;
		auto parse_statement_block() -> utility::result<std::vector<handle<node>>>;

		auto parse_statement() -> utility::result<handle<node>>;

		// expressions
		auto parse_expression() -> utility::result<handle<node>>;
		auto parse_logical_conjunction() -> utility::result<handle<node>>;
		auto parse_logical_disjunction() -> utility::result<handle<node>>;
		auto parse_comparison() -> utility::result<handle<node>>;
		auto parse_term() -> utility::result<handle<node>>;
		auto parse_factor() -> utility::result<handle<node>>;
		auto parse_primary() -> utility::result<handle<node>>;

		auto parse_if_else_statement_block() -> utility::result<handle<node>>;
		auto parse_if_statement() -> utility::result<handle<node>>;

		auto parse_return_statement() -> utility::result<handle<node>>;
		auto parse_identifier_statement() -> utility::result<handle<node>>;
		auto parse_negative_expression() -> utility::result<handle<node>>;

		auto parse_type() -> utility::result<data_type>;
		auto parse_function_call(const std::vector<utility::string_table_key>& namespaces) -> utility::result<handle<node>>;
		auto parse_variable_declaration() -> utility::result<handle<node>>;
		auto parse_variable_access() const-> utility::result<handle<node>>;
		auto parse_assignment() -> utility::result<handle<node>>;

		// literals
		auto parse_numerical_literal() const-> utility::result<handle<node>>;
		auto parse_character_literal() const->utility::result<handle<node>>;
		auto parse_string_literal() const-> utility::result<handle<node>>;
		auto parse_bool_literal() const -> utility::result<handle<node>>;

		auto is_current_token_type() const -> bool;
		auto peek_is_function_definition() -> bool;
		auto peek_is_function_call() const -> bool;
		auto peek_is_variable_declaration() -> bool;
		auto peek_is_namespace_access() -> bool;
		auto peek_is_double_colon() -> bool;

		template<typename extra_type>
		auto create_node(node_type type, u64 child_count) const -> handle<node> {
			return m_context.ast.create_node<extra_type>(type, child_count);
		}

		auto create_binary_expression(node_type type, handle<node> left, handle<node> right) const -> handle<node>;
	private:
		frontend_context& m_context;
		token_buffer_iterator m_tokens;
	};
} // namespace sigma
