#include "dependency_tree.h"

namespace sigma {
	dependency_tree::dependency_tree(
		const filepath& root_path
	) : m_root_compilation_unit_path(absolute(root_path)) {}

	void dependency_tree::print() const {
		m_graph.print();
	}

	outcome::result<void> dependency_tree::verify() const {
		if(m_graph.is_acyclic() == false) {
			return outcome::failure(error::emit<error_code::circular_dependency>());
		}

		return outcome::success();
	}

	outcome::result<void> dependency_tree::construct() {
		// recursively walk the user-defined dependencies and construct the dependency tree
		return construct(m_root_compilation_unit_path);
	}

	u64 dependency_tree::size() const {
		return m_graph.size();
	}

	outcome::result<std::shared_ptr<abstract_syntax_tree>> dependency_tree::parse() {
		// parse individual files
		for(const auto& [filepath, node] : m_graph) {
			OUTCOME_TRY(node->get_value().parse());
		}

		// insert all required abstract syntax trees into their respective parents 
		m_graph.post_order_traverse(
			m_root_compilation_unit_path,
			[&](const auto& path, auto* node) {
				const auto& children = node->get_children();
				auto ast = m_graph[path].get_abstract_syntax_tree();

				u64 offset = 0;

				for (u64 i = 0; i < children.size(); i++) {
					auto ast_to_insert = m_graph[children[i]].get_abstract_syntax_tree();
					const u64 inserted_size = ast_to_insert->size();
					u64 original_index = m_graph[path].get_include_directive_index(i);

					ast->move_insert(
						ast->begin() + original_index + offset,
						ast_to_insert
					);

					offset += inserted_size - 1;
				}
			},
			true
		);

		// return the top abstract syntax tree which now contains all other modules in the correct order
		return m_graph[m_root_compilation_unit_path].get_abstract_syntax_tree();
	}

	outcome::result<void> dependency_tree::construct(
		const filepath& path
	) {
		// skip already visited files
		if (m_graph.contains(path)) {
			return outcome::success();
		}

		OUTCOME_TRY(std::shared_ptr<text_file> file, text_file::load(path));
		OUTCOME_TRY(verify_source_file(file));

		m_file_registry.insert(file, path);

		// tokenize the current file
		lexer lexer(file);
		OUTCOME_TRY(auto token_list, lexer.tokenize());

		const filepath parent_path = path.parent_path();
		std::vector<filepath> includes;

		// traverse the token list and extract all include directive indices 
		while(true) {
			// we've reached the end token, break
			if (token_list.peek_token().get_token() == token::end_of_file) {
				break;
			}

			token_list.synchronize_indices();
			token_list.get_token();

			if(token_list.get_current_token().get_token() == token::hash) {
				OUTCOME_TRY(token_list.expect_token(token::keyword_include));
				OUTCOME_TRY(token_list.expect_token(token::string_literal));

				includes.push_back(
					parent_path / token_list.get_current_token().get_value()
				);
			}
		}

		token_list.set_indices(0); // reset token indices

		// insert a new node for the current file
		m_graph.add_node(
			path, 
			translation_unit(token_list), 
			includes
		);

		// construct dependencies
		for(const auto& include : includes) {
			OUTCOME_TRY(construct(include));
		}
		
		return outcome::success();
	}

	outcome::result<void> dependency_tree::verify_source_file(
		const std::shared_ptr<text_file>& file
	) {
		if (file->get_extension() != LANG_FILE_EXTENSION) {
			return outcome::failure(
				error::emit<error_code::invalid_file_extension>(file->get_path(), LANG_FILE_EXTENSION)
			);
		}

		return outcome::success();
	}

	translation_unit::translation_unit(
		const token_list& token_list
	) : m_token_list(token_list) {}

	outcome::result<void> translation_unit::parse() {
		parser parser(m_token_list);

		OUTCOME_TRY(m_abstract_syntax_tree, parser.parse());
		m_include_directive_indices = parser.get_include_directive_indices();

		return outcome::success();
	}

	std::shared_ptr<abstract_syntax_tree> translation_unit::get_abstract_syntax_tree() const {
		return m_abstract_syntax_tree;
	}

	u64 translation_unit::get_include_directive_index(
		u64 index
	) const {
		return m_include_directive_indices[index];
	}
}
