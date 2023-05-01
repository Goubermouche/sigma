#pragma once
#include "../lexer/lexer.h"
#include "../codegen/llvm_wrappers/type.h"
#include "../codegen/abstract_syntax_tree/abstract_syntax_tree.h"

namespace channel {
	class parser {
	public: 
		parser(const lexer& lexer);
		bool parse();

		const abstract_syntax_tree& get_abstract_syntax_tree();
	private:
		/**
		 * \brief Retrieves the next token from the lexer.
		 */
		void get_next_token();

		/**
		 * \brief Retrieves the next token from the lexer and returns an error if the token does not match the expected token type.
		 * \param token Expected token type
		 * \return True if the two tokens match
		 */
		bool expect_next_token(token token);

		/**
		 * \brief Attempts to parse a function definition. The first expected token is a type.
		 * \param out_node Output AST node
		 * \return True if the expression is parsed successfully
		 */
		bool parse_function_definition(node*& out_node);

		/**
		 * \brief Attempts to parse a global statement.
		 * \param out_node Output AST node
		 * \return True if the expression is parsed successfully
		 */
		bool parse_global_statement(node*& out_node);

		/**
		 * \brief Attempts to parse a local statement.
		 * \param out_node Output AST node
		 * \return True if the expression is parsed successfully
		 */
		bool parse_local_statement(node*& out_node);

		bool parse_local_statement_identifier(node*& out_node);

		/**
		 * \brief Attempts to parse an if else chain. The first expected token is the keyword_if token.
		 * \param out_node Output AST node
		 * \return True if the expression is parsed successfully
		 */
		bool parse_if_else_statement(node*& out_node);

		bool parse_while_loop(node*& out_node);

		bool parse_loop_increment(node*& out_node);

		bool parse_for_loop(node*& out_node);

		/**
		 * \brief Attempts to parse an assignment operation. The first expected token is an identifier.
		 * \param out_node Output AST node
		 * \return True if the expression is parsed successfully
		 */
		bool parse_assignment(node*& out_node);

		/**
		 * \brief Attempts to parse an array assignment. The first expected token is an identifier.
		 * \param out_node Output AST node
		 * \return True if the expression is parsed successfully
		 */
		bool parse_array_assignment(node*& out_node);

		/**
		 * \brief Attempts to parse an array access. The first expected token is an identifier.
		 * \param out_node Output AST node
		 * \return True if the expression is parsed successfully
		 */
		bool parse_array_access(node*& out_node);

		/**
		 * \brief Attempts to parse a function call. The first expected token is an identifier.
		 * \param out_node Output AST node
		 * \return True if the expression is parsed successfully
		 */
		bool parse_function_call(node*& out_node);

		/**
		 * \brief Attempts to parse a return statement. The first expected token is a return keyword.
		 * \param out_node Output AST node
		 * \return True if the expression is parsed successfully
		 */
		bool parse_return_statement(node*& out_node);

		/**
		 * \brief Attempts to parse a variable declaration. The first expected token is a type.
		 * \param out_node Output AST node
		 * \param is_global Global setting of the declaration (global vs. local declaration)
		 * \return True if the expression is parsed successfully
		 */
		bool parse_declaration(node*& out_node, bool is_global);

		/**
		 * \brief Attempts to parse an expression. 
		 * \param out_node Output AST node
		 * \param expression_type Specified expression type
		 * \return True if the expression is parsed successfully
		 */
		bool parse_expression(node*& out_node, type expression_type = type::unknown());

		/**
		 * \brief Attempts to parse a conjunction expression node.
		 * \param out_node Output AST node
		 * \param expression_type Specified expression type
		 * \return True if the expression is parsed successfully
		 */
		bool parse_logical_conjunction(node*& out_node, type expression_type);

		/**
		 * \brief Attempts to parse a disjunction expression node.
		 * \param out_node Output AST node
		 * \param expression_type Specified expression type
		 * \return True if the expression is parsed successfully
		 */
		bool parse_logical_disjunction(node*& out_node, type expression_type);

		/**
		 * \brief Attempts to parse a comparison expression node.
		 * \param out_node Output AST node
		 * \param expression_type Specified expression type
		 * \return True if the expression is parsed successfully
		 */
		bool parse_comparison(node*& out_node, type expression_type);

		/**
		 * \brief Attempts to parse a term expression node.
		 * \param out_node Output AST node
		 * \param expression_type Specified expression type
		 * \return True if the expression is parsed successfully
		 */
		bool parse_term(node*& out_node, type expression_type);

		/**
		 * \brief Attempts to parse a factor expression node.
		 * \param out_node Output AST node
		 * \param expression_type Specified expression type
		 * \return True if the expression is parsed successfully
		 */
		bool parse_factor(node*& out_node, type expression_type);

		bool parse_pre_operator(node*& out_node);
		bool parse_post_operator(node* operand, node*& out_node);

		/**
		 * \brief Attempts to parse a primary expression node.
		 * \param out_node Output AST node
		 * \param expression_type Specified expression type
		 * \return True if the expression is parsed successfully
		 */
		bool parse_primary(node*& out_node, type expression_type);

		/**
		 * \brief Attempts to parse a number token. The first expected token is a numerical type.
		 * \param out_node Output AST node
		 * \param expression_type Specified expression type
		 * \return True if the expression is parsed successfully
		 */
		bool parse_number(node*& out_node, type expression_type = type::unknown());

		/**
		 * \brief Attempts to parse a char literal. The first expected token is a char_literal.
		 * \param out_node Output AST node
		 * \return True if the expression is parsed successfully
		 */
		bool parse_char(node*& out_node);

		/**
		 * \brief Attempts to parse a string literal. The first expected token is a string_literal.
		 * \param out_node Output AST node
		 * \return True if the expression is parsed successfully
		 */
		bool parse_string(node*& out_node);

		/**
		 * \brief Attempts to parse a boolean value. The first expected token is either a bool_literal_true or a bool_literal_false.
		 * \param out_node Output AST node
		 * \return True if the expression is parsed successfully
		 */
		bool parse_bool(node*& out_node);

		bool parse_break_keyword(node*& out_node);

		/**
		 * \brief Parses a negative number. The first expected token is a minus operator.
		 * \param out_node Output AST node
		 * \param expression_type Specified expression type
		 * \return True if the expression is parsed successfully
		 */
		bool parse_negative_number(node*& out_node, type expression_type = type::unknown());

		/**
		 * \brief Parses a new allocation. The first expected token is the 'new' keyword.
		 * \param out_node Output AST node
		 * \return True if the expression is parsed successfully
		 */
		bool parse_new_allocation(node*& out_node);

		/**
		 * \brief Parses either a function call or a variable assignment. The first expected token is an identifier.
		 * \param out_node Output AST node
		 * \return True if the expression is parsed successfully
		 */
		bool parse_primary_identifier(node*& out_node);

		/**
		 * \brief Parses a deep (nested) expression. The first expected token is a left parenthesis. 
		 * \param out_node Output AST node
		 * \param expression_type Specified expression type
		 * \return True if the expression is parsed successfully
		 */
		bool parse_deep_expression(node*& out_node, type expression_type);

		/**
		 * \brief Checks if a statement is a function definition. The first expected token is a type.
		 * \return True if the statement is a function definition
		 */
		bool peek_is_function_definition();

		/**
		 * \brief Checks if the next statement is a function call.
		 * \return True if the next expression is a function call
		 */
		bool peek_is_function_call();

		bool peek_is_assignment();

		/**
		 * \brief Checks if the next statement is an array index access.
		 * \return True if the next statement is an array index access.
		 */
		bool peek_is_array_index_access();

		bool peek_is_post_operator();

		/**
		 * \brief Returns the next token without advancing the inner lexer.
		 * \return Returns the next token
		 */
		token peek_next_token();

		token peek_nth_token(u64 offset);

		/**
		 * \brief Creates a new numerical node with the value of '0'.
		 * \param expression_type Numerical type
		 * \return Expression of the specified with the value of '0'
		 */
		node* create_zero_node(type expression_type) const;

		/**
		 * \brief Parses the next tokens as a type. The first expected token is a type.
		 * \return Parsed type token
		 */
		bool parse_type(type& ty);
	private:
		lexer m_lexer;
		token_data m_current_token;
		abstract_syntax_tree m_abstract_syntax_tree;
	};
}
