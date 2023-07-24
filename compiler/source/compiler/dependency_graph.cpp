#include "dependency_graph.h"

namespace sigma {
	dependency_graph::dependency_graph(
		const filepath& root_path
	) : m_root_compilation_unit_path(absolute(root_path)) {}

	void dependency_graph::print() const {
		m_graph.print();
	}

	outcome::result<void> dependency_graph::verify() const {
		if(m_graph.is_acyclic() == false) {
			return outcome::failure(error::emit<5100>());
		}

		return outcome::success();
	}

	outcome::result<void> dependency_graph::construct() {
		return construct(m_root_compilation_unit_path);
	}

	u64 dependency_graph::size() const {
		return m_graph.size();
	}

	outcome::result<std::shared_ptr<abstract_syntax_tree>> dependency_graph::parse() {
		// todo: cleanup
		std::unordered_map<filepath, std::shared_ptr<abstract_syntax_tree>> abstract_syntax_trees;
		std::unordered_map<filepath, std::vector<u64>> include_directive_indices;

		// parse individual files
		for(const auto& [filepath, node] : m_graph) {
			parser parser(node->get_value());
			OUTCOME_TRY(abstract_syntax_trees[filepath], parser.parse());
			include_directive_indices[filepath] = parser.get_include_directive_indices();
		}

		// insert all required abstract syntax trees into their respective parents 
		m_graph.post_order_traverse(
			m_root_compilation_unit_path,
			[&](const auto& path, auto* node) {
				const auto& children = node->get_children();
				auto ast = abstract_syntax_trees[path];
				u64 offset = 0;

				for (u64 i = 0; i < children.size(); i++) {
					auto ast_to_insert = abstract_syntax_trees[children[i]];
					const u64 inserted_size = ast_to_insert->size();
					u64 original_index = include_directive_indices[path][i];

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
		return abstract_syntax_trees[m_root_compilation_unit_path];

	}

	outcome::result<void> dependency_graph::construct(
		const filepath& path
	) {
		if (m_graph.contains(path)) {
			return outcome::success();
		}

		OUTCOME_TRY(verify_source_file(path));

		lexer lexer;
		lexer.set_source_filepath(path);
		lexer.tokenize();
		auto token_list = lexer.get_token_list();
		const filepath parent_path = path.parent_path();
		std::vector<filepath> includes;

		// traverse until we run out of preprocessor directives
		while(true) {
			if(token_list.peek_token().get_token() == token::hash) {
				token_list.synchronize_indices();

				token_list.get_token(); // hash (guaranteed)
				OUTCOME_TRY(token_list.expect_token(token::keyword_include));
				OUTCOME_TRY(token_list.expect_token(token::string_literal));

				includes.push_back(
					parent_path / token_list.get_current_token().get_value()
				);
			}
			else {
				// stop processing other tokens
				// note: this may include other include directives which are located lower,
				//       these will be caught by the parser and an error will be thrown,
				//       this is expected behaviour
				break;
			}
		}

		token_list.set_indices(0); // reset token indices
		m_graph.add_node(path, token_list, includes);

		for(const auto& include : includes) {
			OUTCOME_TRY(construct(include));
		}

		return outcome::success();
	}

	outcome::result<void> dependency_graph::verify_source_file(
		const filepath& path
	) {
		if (!exists(path)) {
			return outcome::failure(error::emit<1002>(path));
		}

		if (!detail::is_file(path)) {
			return outcome::failure(error::emit<1003>(path));
		}

		if (detail::extract_extension_from_filepath(path) != LANG_EXTENSION) {
			return outcome::failure(error::emit<1007>(path, LANG_EXTENSION));
		}

		return outcome::success();
	}
}
