#pragma once
#include "../llvm_wrappers/value.h"
#include <llvm/IR/Value.h>
#include "../../utility/macros.h"

namespace channel {
	class visitor;

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
		virtual bool accept(visitor& visitor, value*& out_value) = 0;
		virtual void print(int depth, const std::wstring& prefix, bool is_last) = 0;
		u64 get_declaration_line_number() const;
	protected:
		static void print_value(int depth, const std::wstring& prefix, const std::wstring& node_name, const std::string& value, bool is_last);
		static std::wstring get_new_prefix(int depth, const std::wstring& prefix, bool is_last);
	private:
		u64 m_line_number = 0;
	};
}

#include "../visitor/visitor.h"