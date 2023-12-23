#pragma once
#include "parser/data_type.h"

#include <tokenizer/token_buffer.h>
#include <abstract_syntax_tree/abstract_syntax_tree.h>

namespace sigma {
	using namespace utility::types;

	struct compilation_context;

	class parser {
	public:
		[[nodiscard]] static auto parse(compilation_context& context) -> utility::result<abstract_syntax_tree>;
	private:
		parser(compilation_context& context);
		[[nodiscard]] auto parse() -> utility::result<abstract_syntax_tree>;

		auto parse_function_declaration() -> utility::result<handle<node>>;

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

		auto parse_type() const -> utility::result<data_type>;
		auto parse_function_call() -> utility::result<handle<node>>;
		auto parse_variable_declaration() -> utility::result<handle<node>>;
		auto parse_variable_access() -> utility::result<handle<node>>;
		auto parse_assignment() -> utility::result<handle<node>>;

		// literals
		auto parse_numerical_literal() -> utility::result<handle<node>>;
		auto parse_string_literal() -> utility::result<handle<node>>;
		auto parse_bool_literal() -> utility::result<handle<node>>;

		auto is_current_token_type() const -> bool;
		auto peek_is_function_definition() -> bool;
		auto peek_is_function_call() const -> bool;
		auto peek_is_variable_declaration() const -> bool;
	private:
		compilation_context& m_context;
		token_buffer_iterator m_tokens;
		abstract_syntax_tree m_ast;
	};
} // namespace sigma
