#pragma once
#include "parser/data_type.h"

#include <tokenizer/token_buffer.h>
#include <abstract_syntax_tree/abstract_syntax_tree.h>

namespace sigma {
	using namespace utility::types;

	struct compilation_context;

	class parser {
	public:
		[[nodiscard]] static auto parse(compilation_context& context) -> abstract_syntax_tree;
	private:
		parser(compilation_context& context);
		[[nodiscard]] auto parse() -> abstract_syntax_tree;

		auto parse_function_declaration() -> handle<node>;

		auto parse_statement_block() -> std::vector<handle<node>>;
		auto parse_statement() -> handle<node>;

		// expressions
		auto parse_expression() -> handle<node>;
		auto parse_logical_conjunction() -> handle<node>;
		auto parse_logical_disjunction() -> handle<node>;
		auto parse_comparison() -> handle<node>;
		auto parse_term() -> handle<node>;
		auto parse_factor() -> handle<node>;
		auto parse_primary() -> handle<node>;

		auto parse_if_else_statement_block() -> handle<node>;
		auto parse_if_statement() -> handle<node>;

		auto parse_return_statement() -> handle<node>;
		auto parse_identifier_statement() -> handle<node>;
		auto parse_negative_expression() -> handle<node>;

		auto parse_type() const -> data_type;
		auto parse_function_call() -> handle<node>;
		auto parse_variable_declaration() -> handle<node>;
		auto parse_variable_access() -> handle<node>;
		auto parse_assignment() -> handle<node>;

		// literals
		auto parse_numerical_literal() -> handle<node>;
		auto parse_string_literal() -> handle<node>;
		auto parse_bool_literal() -> handle<node>;

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
