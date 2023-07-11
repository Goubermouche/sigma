#pragma once
#include "llvm_wrappers/value.h"
#include "llvm_wrappers/code_generation_context.h"
#include "compiler/diagnostics/error.h"
#include "compiler/diagnostics/warning.h"

#define AST_NODE_NUMERICAL_LITERAL_COLOR color::blue
#define AST_NODE_BOOLEAN_LITERAL_COLOR	 color::blue
#define AST_NODE_TEXT_LITERAL_COLOR		 color::orange
#define AST_NODE_TYPE_COLOR				 color::light_blue
#define AST_NODE_OPERATOR_COLOR			 color::magenta
#define AST_NODE_VARIABLE_COLOR			 color::green

namespace sigma {
	class code_generator;
	class node;

	using node_ptr = node*;
	using expected_value = std::expected<value_ptr, error_msg>;

	/**
	 * \brief base AST node.
	 */
	class node {
	public:
		/**
		 * \brief Constructs a node with using data about the node's location.
		 * \param location Token location of the node. 
		 */
		node(const file_position& location);
		virtual ~node() = default;

		/**
		 * \brief Accepts the node and implements the required operations for the specified node type.
		 * \param visitor Visitor to use for implementing the needed operations
		 * \param context Context provided by the previous accept call
		 * \return Result of the acceptation operation, either an error or a pointer to the generated value.
		 */
		virtual expected_value accept(
			code_generator& visitor,
			const code_generation_context& context
		) = 0;

		/**
		 * \brief Prints the given node as a part of a tree hierarchy.
		 * \param depth Depth of the given node
		 * \param prefix Prefix string used for determining the tree visualization
		 * \param is_last Determines whether the current node is the last node in the current tree branch
		 */
		virtual void print(
			u64 depth,
			const std::wstring& prefix,
			bool is_last
		) = 0;

		/**
		 * \brief Gets the declared node location.
		 * \return Declared node location
		 */
		const file_position& get_declared_location() const;
	protected:
		/**
		 * \brief Prints a node name with additional formatting relevant to the tree hierarchy.
		 * \param depth Depth of the given node
		 * \param prefix Prefix string used for determining the tree visualization
		 * \param node_name Node name to print
		 * \param is_last Determines whether the current node is the last node in the current tree branch
		 */
		static void print_node_name(
			u64 depth, 
			const std::wstring& prefix,
			const std::string& node_name,
			bool is_last
		);

		/**
		 * \brief Generates a new tree prefix for the given node.
		 * \param depth Depth of the given node
		 * \param prefix Prefix string used for determining the tree visualization
		 * \param is_last Determines whether the current node is the last node in the current tree branch
		 * \return New prefix which accounts for increased depth.
		 */
		static std::wstring get_new_prefix(
			u64 depth, 
			const std::wstring& prefix,
			bool is_last
		);
	private:
		file_position m_location;
	};
}

#include "code_generator/code_generator.h"
