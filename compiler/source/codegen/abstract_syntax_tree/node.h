#pragma once
#include "codegen/llvm_wrappers/value.h"
#include "codegen/llvm_wrappers/codegen_context.h"

#define AST_NODE_NUMERICAL_LITERAL_COLOR color::blue
#define AST_NODE_BOOLEAN_LITERAL_COLOR	 color::blue
#define AST_NODE_TEXT_LITERAL_COLOR		 color::orange
#define AST_NODE_TYPE_COLOR				 color::light_blue
#define AST_NODE_OPERATOR_COLOR			 color::magenta
#define AST_NODE_VARIABLE_COLOR			 color::green

namespace channel {
	class visitor;
	class node;

	using node_ptr = node*;

	/**
	 * \brief base AST node.
	 */
	class node {
	public:
		node(const token_position& position);
		virtual ~node() = default;

		/**
		 * \brief Accepts the given \a visitor and handles the given node.
		 * \param visitor Visitor to accept
		 * \param out_value Resulting value
		 * \param context Acceptation context
		 * \returns True if no error has been met, otherwise false
		 */
		virtual bool accept(
			visitor& visitor,
			value_ptr& out_value,
			codegen_context context
		) = 0;

		virtual void print(
			int depth,
			const std::wstring& prefix,
			bool is_last
		) = 0;

		const token_position& get_declared_position() const;
	protected:
		static void print_value(
			int depth, 
			const std::wstring& prefix,
			const std::string& node_name,
			bool is_last
		);

		static std::wstring get_new_prefix(
			int depth, 
			const std::wstring& prefix,
			bool is_last
		);
	private:
		token_position m_position;
	};
}

#include "../visitor/visitor.h"