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
#include <compiler/compiler/type_system/data_type.h>
#include <compiler/compiler/compilation_context.h>
#include <abstract_syntax_tree/tree.h>
#include <tokenizer/token_buffer.h>

namespace sigma {
	using namespace utility::types;

	struct frontend_context;

	class parser {
	public:
		[[nodiscard]] static auto parse(frontend_context& context) -> utility::result<void>;
	private:
		using parse_result = utility::result<handle<ast::node>>;
		using parse_block_result = utility::result<std::vector<handle<ast::node>>>;

		parser(frontend_context& context);
		[[nodiscard]] auto parse() -> utility::result<void>;

		// declaration
		auto parse_namespace_declaration() -> parse_result;
		auto parse_function_declaration() -> parse_result;
		auto parse_variable_declaration() -> parse_result;

		// blocks
		auto parse_if_else_statement_block() -> parse_result;
		auto parse_namespace_block() -> parse_block_result;
		auto parse_statement_block() -> parse_block_result;

		// literals
		auto parse_numerical_literal() -> parse_result;
		auto parse_character_literal() -> parse_result;
		auto parse_string_literal() -> parse_result;
		auto parse_bool_literal() -> parse_result;
		auto parse_null_literal() -> parse_result;

		// expressions
		auto parse_identifier_expression() -> parse_result;
		auto parse_logical_conjunction() -> parse_result;
		auto parse_logical_disjunction() -> parse_result;
		auto parse_negative_expression() -> parse_result;
		auto parse_logical_not_expression() -> parse_result;
		auto parse_expression() -> parse_result;
		auto parse_comparison() -> parse_result;
		auto parse_primary() -> parse_result;
		auto parse_factor() -> parse_result;
		auto parse_term() -> parse_result;

		// statements
		auto parse_identifier_statement() -> parse_result;
		auto parse_return_statement() -> parse_result;
		auto parse_if_statement() -> parse_result;
		auto parse_statement() -> parse_result;

		// loads / stores
		auto parse_variable_access() -> parse_result;
		auto parse_array_access() -> parse_result;
		auto parse_assignment() -> parse_result;

		// utility
		auto parse_namespaces() -> utility::result<std::vector<utility::string_table_key>>;
		auto parse_type() -> utility::result<data_type>;

		// other
		auto parse_function_call(const std::vector<utility::string_table_key>& namespaces) -> parse_result;
		auto parse_explicit_cast() -> parse_result;
		auto parse_sizeof() -> parse_result;

		// peeks
		auto peek_is_variable_declaration() -> bool;
		auto peek_is_function_definition() -> bool;
		auto peek_is_explicit_cast() const -> bool;
		auto peek_is_function_call() const -> bool;
		auto peek_is_namespace_access() -> bool;

		// utility
		auto is_current_token_type() const -> bool;
		auto get_current_location() const -> handle<token_location>;

		template<typename extra_type = utility::empty_property>
		auto create_node(ast::node_type type, u64 child_count, handle<token_location> location) const -> handle<ast::node> {
			return m_context.syntax.ast.create_node<extra_type>(type, child_count, location);
		}

		auto create_variable_declaration(u64 child_count, handle<token_location> location) const -> handle<ast::node>;
		auto create_function_call(u64 child_count, handle<token_location> location) const -> handle<ast::node>;
		auto create_array_access(u64 child_count, handle<token_location> location) const -> handle<ast::node>;
		auto create_namespace(u64 child_count, handle<token_location> location) const -> handle<ast::node>;
		auto create_function(u64 child_count, handle<token_location> location) const -> handle<ast::node>;
		auto create_return(u64 child_count, handle<token_location> location) const -> handle<ast::node>;

		auto create_numerical_literal(handle<token_location> location) const -> handle<ast::node>;
		auto create_character_literal(handle<token_location> location) const -> handle<ast::node>;
		auto create_variable_access(handle<token_location> location) const -> handle<ast::node>;
		auto create_string_literal(handle<token_location> location) const -> handle<ast::node>;
		auto create_bool_literal(handle<token_location> location) const -> handle<ast::node>;
		auto create_null_literal(handle<token_location> location) const -> handle<ast::node>;
		auto create_assignment(handle<token_location> location) const -> handle<ast::node>;
		auto create_sizeof(handle<token_location> location) const -> handle<ast::node>;
		auto create_cast(handle<token_location> location) const -> handle<ast::node>;
		auto create_logical_not(handle<token_location> location) const -> handle<ast::node>;

		auto create_comparison_operation(ast::node_type type, handle<ast::node> left, handle<ast::node> right) const -> handle<ast::node>;
		auto create_binary_operation(ast::node_type type, handle<ast::node> left, handle<ast::node> right) const->handle<ast::node>;

		auto create_conditional_branch(u64 child_count) const->handle<ast::node>;
		auto create_branch(u64 child_count) const->handle<ast::node>;

	private:
		frontend_context& m_context;
		token_buffer_iterator m_tokens;
	};
} // namespace sigma
