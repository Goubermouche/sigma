// Root parser file, contains a basic recursive descent parser implementation
//
// -   Whenever we're parsing the first expected token is located at 'm_tokens.get_current_token()',
//     after we're done parsing, we also have to prime the next token using 'm_tokens.next()'.
// -   Three main parse 'operations'
//     1.   Final operation - an operation which does not invoke any other operations
//     2.   Operation group - a set of other operation groups / final operations.
//     3.   Peek operations - looks ahead in the token list and determines if a token sequence is
//          present or not. Does not throw errors

#pragma once
#include "parser/data_type.h"

#include <tokenizer/token_buffer.h>
#include <compiler/compiler/compilation_context.h>
#include <abstract_syntax_tree/abstract_syntax_tree.h>

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
		auto parse_identifier_expression() -> utility::result<handle<node>>;
		auto parse_identifier_statement() -> utility::result<handle<node>>;
		auto parse_negative_expression() -> utility::result<handle<node>>;
		auto parse_explicit_cast() -> utility::result<handle<node>>;
		auto parse_sizeof() -> utility::result<handle<node>>;

		auto parse_type() -> utility::result<data_type>;
		auto parse_function_call(const std::vector<utility::string_table_key>& namespaces) -> utility::result<handle<node>>;
		auto parse_variable_declaration() -> utility::result<handle<node>>;
		auto parse_variable_access() -> utility::result<handle<node>>;
		auto parse_assignment() -> utility::result<handle<node>>;

		// literals
		auto parse_numerical_literal() -> utility::result<handle<node>>;
		auto parse_character_literal() -> utility::result<handle<node>>;
		auto parse_string_literal() -> utility::result<handle<node>>;
		auto parse_bool_literal() -> utility::result<handle<node>>;

		// peeks 
		auto peek_is_function_definition() -> bool;
		auto peek_is_explicit_cast() const -> bool;
		auto peek_is_function_call() const -> bool;
		auto peek_is_variable_declaration() -> bool;
		auto peek_is_namespace_access() -> bool;

		// utility
		auto is_current_token_type() const -> bool;

		template<typename extra_type>
		auto create_node(node_type type, u64 child_count, handle<token_location> location) const -> handle<node> {
			return m_context.syntax.ast.create_node<extra_type>(type, child_count, location);
		}

		auto create_binary_expression(node_type type, handle<node> left, handle<node> right) const -> handle<node>;
	private:
		frontend_context& m_context;
		token_buffer_iterator m_tokens;
	};
} // namespace sigma
