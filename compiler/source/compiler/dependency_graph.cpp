#include "dependency_graph.h"
#include "utility/filesystem.h"

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

	outcome::result<void> dependency_graph::traverse_compile(
		detail::thread_pool& pool
	) {
		std::atomic error_encountered = false;
		std::mutex error_mutex;
		std::shared_ptr<error_message> error;

		m_graph.traverse_parallel_bottom_up(
			m_root_compilation_unit_path,
			pool,
			[&](const filepath& path, auto* node) {
				// stop compiling if we've encountered an error
				if(error_encountered) {
					return;
				}

				// at this point all children have been compiled
				// gather dependency contexts
				const auto& child_nodes = node->get_children();

				std::vector<std::shared_ptr<llvm_context>> dependencies(
					child_nodes.size()
				);

				for (u64 i = 0; i < child_nodes.size(); i++) {
					dependencies[i] = m_graph.get_value(child_nodes[i]).get_context();
				}

				// compile the current unit
				console::out
					<< "compiling file: "
					<< color::light_blue
					<< path
					<< '\n'
					<< color::white;

				const auto compilation_result = node->get_value().compile(dependencies);

				if(compilation_result.has_error()) {
					error_encountered = true;
					std::lock_guard error_lock(error_mutex);
					error = compilation_result.get_error();
				}
			}
		);

		if(error_encountered) {
			return outcome::failure(error);
		}

		return outcome::success();
	}

	outcome::result<void> dependency_graph::construct() {
		return construct(m_root_compilation_unit_path);
	}

	outcome::result<void> dependency_graph::construct(
		const filepath& path
	) {
		if (m_graph.contains(path)) {
			return outcome::success();
		}

		OUTCOME_TRY(auto file_contents, detail::read_file(path));
		const std::regex include_regex(R"((^|\n)\s*#include\s*\"([^\"]+)\")");
		const filepath parent_path = path.parent_path();
		std::vector<filepath> includes;
		std::string remainder = file_contents;
		std::smatch match;

		bool include_found = false;
		while (std::regex_search(remainder, match, include_regex)) {
			if (include_found && !match.prefix().str().empty() && match.prefix().str().find_first_not_of(" \t\n") != std::string::npos) {
				const u64 line_index = std::count(file_contents.begin(), file_contents.begin() + file_contents.size() - remainder.size(), '\n') + 1;
				const u64 char_index = file_contents.size() - remainder.size() - file_contents.rfind('\n', file_contents.size() - remainder.size());

				return outcome::failure(
					error::emit<5101>({ path, line_index, char_index })
				);
			}

			includes.push_back(parent_path / match[2].str());
			remainder = match.suffix().str();
			include_found = true;
		}

		compilation_unit unit;
		m_graph.add_node(path, unit, includes);

		for(const auto& include : includes) {
			OUTCOME_TRY(construct(include));
		}

		return outcome::success();
	}
}
