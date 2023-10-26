#pragma once
#include "lexer/lexer.h"
#include "abstract_syntax_tree/abstract_syntax_tree.h"
#include "abstract_syntax_tree/llvm_wrappers/type.h"

namespace sigma {
	class parser {
	public:
		parser(const token_list& token_list);
		virtual ~parser() = default;

		utility::outcome::result<s_ptr<abstract_syntax_tree>> parse();
		s_ptr<abstract_syntax_tree> get_abstract_syntax_tree() const;
		const std::vector<u64>& get_include_directive_indices() const;
	private:
		utility::outcome::result<node_ptr> parse_function_definition();
		utility::outcome::result<node_ptr> parse_global_statement();
		utility::outcome::result<void> parse_include_directive();
		utility::outcome::result<std::vector<node_ptr>> parse_local_statements();
		utility::outcome::result<node_ptr> parse_local_statement();
		utility::outcome::result<node_ptr> parse_local_statement_identifier();
		utility::outcome::result<node_ptr> parse_if_else_statement();
		utility::outcome::result<node_ptr> parse_while_loop();
		utility::outcome::result<node_ptr> parse_loop_increment();
		utility::outcome::result<node_ptr> parse_for_loop();
		utility::outcome::result<node_ptr> parse_compound_operation(
			node_ptr left_operand
		);

		utility::outcome::result<node_ptr> parse_variable_access();
		utility::outcome::result<node_ptr> parse_variable();
		utility::outcome::result<node_ptr> parse_assignment();
		utility::outcome::result<node_ptr> parse_array_assignment();
		utility::outcome::result<node_ptr> parse_array_access();
		utility::outcome::result<node_ptr> parse_function_call();
		utility::outcome::result<node_ptr> parse_return_statement();
		utility::outcome::result<node_ptr> parse_declaration(
			bool is_global
		);

		utility::outcome::result<node_ptr> parse_expression(
			type expression_type = type::unknown()
		);

		utility::outcome::result<node_ptr> parse_logical_conjunction(
			type expression_type
		);

		utility::outcome::result<node_ptr> parse_logical_disjunction(
			type expression_type
		);

		utility::outcome::result<node_ptr> parse_comparison(
			type expression_type
		);

		utility::outcome::result<node_ptr> parse_term(
			type expression_type
		);

		utility::outcome::result<node_ptr> parse_factor(
			type expression_type
		);

		utility::outcome::result<node_ptr> parse_pre_operator();

		utility::outcome::result<node_ptr> parse_post_operator(
			node_ptr operand
		);

		utility::outcome::result<node_ptr> parse_primary(
			type expression_type
		);

		utility::outcome::result<node_ptr> parse_number(
			type expression_type = type::unknown()
		);

		utility::outcome::result<node_ptr> parse_char();
		utility::outcome::result<node_ptr> parse_string();
		utility::outcome::result<node_ptr> parse_bool();
		utility::outcome::result<node_ptr> parse_break_keyword();

		utility::outcome::result<node_ptr> parse_negative_number(
			type expression_type = type::unknown()
		);

		utility::outcome::result<node_ptr> parse_new_allocation();
		utility::outcome::result<node_ptr> parse_primary_identifier();
		utility::outcome::result<node_ptr> parse_deep_expression(
			type expression_type
		);

		bool peek_is_function_definition();
		bool peek_is_function_call();
		bool peek_is_assignment();
		bool peek_is_array_index_access();
		bool peek_is_post_operator();
		bool peek_is_include_directive();

		token peek_next_token();
		token peek_nth_token(
			u64 offset
		);

		node_ptr create_zero_node(
			type expression_type
		) const;

		utility::outcome::result<type> parse_type();
	private:
		token_list m_token_list;
		s_ptr<abstract_syntax_tree> m_abstract_syntax_tree;
		std::vector<u64> m_include_directive_indices;
		u64 m_top_level_abstract_syntax_tree_node_count = 0;
	};
}
