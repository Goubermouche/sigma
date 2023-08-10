#pragma once
#include "parser/parser.h"
#include "utility/diagnostics/error.h"
#include "utility/containers/directed_graph.h"

namespace sigma {
	class translation_unit {
	public:
		translation_unit() = default;
		translation_unit(const token_list& token_list);

		outcome::result<void> parse();

		std::shared_ptr<abstract_syntax_tree> get_abstract_syntax_tree() const;
		u64 get_include_directive_index(u64 index) const;
	private:
		token_list m_token_list;
		std::vector<u64> m_include_directive_indices;
		std::shared_ptr<abstract_syntax_tree> m_abstract_syntax_tree;
	};

	class dependency_tree {
	public:
		dependency_tree(const filepath& root_path);

		/**
		 * \brief Constructs the dependency tree and traverses all user-defined dependencies.
		 * \return outcome (void)
		 */
		outcome::result<void> construct();

		/**
		 * \brief Parses all contained compilation units and creates the parent abstract syntax tree.
		 * \return outcome (abstract syntax tree containing all relevant dependencies in the user-defined order)
		 */
		outcome::result<std::shared_ptr<abstract_syntax_tree>> parse();

		/**
		 * \brief Verifies the dependency tree structure and checks for any circular dependencies. 
		 * \return outcome (void)
		 */
		outcome::result<void> verify() const;

		u64 size() const;
		void print() const;
	private:
		outcome::result<void> construct(const filepath& path);

		/**
		 * \brief Checks the validity of the given source file, this includes checking if the file exists, if it even is a file,
		 * and if the file extension matches the .s standard. 
		 * \param path Path of the source file to verify
		 * \return outcome (void)
		 */
		static outcome::result<void> verify_source_file(
			const std::shared_ptr<text_file>& file
		);
	private:
		filepath m_root_compilation_unit_path;
		detail::directed_graph<filepath, translation_unit> m_graph;
	};
}
