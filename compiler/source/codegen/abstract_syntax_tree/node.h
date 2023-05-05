#pragma once
#include "../llvm_wrappers/value.h"
#include <llvm/IR/Value.h>
#include "../../utility/macros.h"

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
		node(u64 line_number);
		virtual ~node() = default;

		/**
		 * \brief Accepts the given \a visitor and handles the given node.
		 * \param visitor Visitor to accept
		 * \param out_value Resulting value
		 * \returns True if no error has been met, otherwise false
		 */
		virtual bool accept(visitor& visitor, value_ptr& out_value) = 0;
		virtual void print(int depth, const std::wstring& prefix, bool is_last) = 0;
		u64 get_declaration_line_number() const;
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
		u64 m_line_number = 0;
	};
}

#include "../visitor/visitor.h"