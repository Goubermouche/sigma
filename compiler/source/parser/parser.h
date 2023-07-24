#pragma once
#include "lexer/lexer.h"
#include "code_generator/abstract_syntax_tree/abstract_syntax_tree.h"
#include "llvm_wrappers/type.h"

namespace sigma {
	class parser {
	public:
		parser(const token_list& token_list);
		virtual ~parser() = default;

		outcome::result<std::shared_ptr<abstract_syntax_tree>> parse();
		std::shared_ptr<abstract_syntax_tree> get_abstract_syntax_tree() const;
		const std::vector<u64>& get_include_directive_indices() const;
	private:
		outcome::result<node_ptr> parse_function_definition();
		outcome::result<node_ptr> parse_global_statement();
		outcome::result<void> parse_include_directive();
		outcome::result<std::vector<node_ptr>> parse_local_statements();
		outcome::result<node_ptr> parse_local_statement();
		outcome::result<node_ptr> parse_local_statement_identifier();
		outcome::result<node_ptr> parse_if_else_statement();
		outcome::result<node_ptr> parse_while_loop();
		outcome::result<node_ptr> parse_loop_increment();
		outcome::result<node_ptr> parse_for_loop();
		outcome::result<node_ptr> parse_compound_operation(
			node_ptr left_operand
		);

		outcome::result<node_ptr> parse_assignment();
		outcome::result<node_ptr> parse_array_assignment();
		outcome::result<node_ptr> parse_array_access();
		outcome::result<node_ptr> parse_function_call();
		outcome::result<node_ptr> parse_return_statement();
		outcome::result<node_ptr> parse_declaration(
			bool is_global
		);

		outcome::result<node_ptr> parse_expression(
			type expression_type = type::unknown()
		);

		outcome::result<node_ptr> parse_logical_conjunction(
			type expression_type
		);

		outcome::result<node_ptr> parse_logical_disjunction(
			type expression_type
		);

		outcome::result<node_ptr> parse_comparison(
			type expression_type
		);

		outcome::result<node_ptr> parse_term(
			type expression_type
		);

		outcome::result<node_ptr> parse_factor(
			type expression_type
		);

		outcome::result<node_ptr> parse_pre_operator();

		outcome::result<node_ptr> parse_post_operator(
			node_ptr operand
		);

		outcome::result<node_ptr> parse_primary(
			type expression_type
		);

		outcome::result<node_ptr> parse_number(
			type expression_type = type::unknown()
		);

		outcome::result<node_ptr> parse_char();
		outcome::result<node_ptr> parse_string();
		outcome::result<node_ptr> parse_bool();
		outcome::result<node_ptr> parse_break_keyword();

		outcome::result<node_ptr> parse_negative_number(
			type expression_type = type::unknown()
		);

		outcome::result<node_ptr> parse_new_allocation();
		outcome::result<node_ptr> parse_primary_identifier();
		outcome::result<node_ptr> parse_deep_expression(
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

		outcome::result<type> parse_type();
	private:
		token_list m_token_list;
		std::shared_ptr<abstract_syntax_tree> m_abstract_syntax_tree;
		std::vector<u64> m_include_directive_indices;
		u64 m_top_level_abstract_syntax_tree_node_count = 0;
	};
}
