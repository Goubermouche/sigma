#pragma once
#include "parser/parser.h"
#include "utility/diagnostics/error.h"
#include "utility/thread_pool.h"
#include "utility/containers/directed_graph.h"

namespace sigma {
	class dependency_graph {
	public:
		dependency_graph(const filepath& root_path);

		void print() const;
		outcome::result<void> verify() const;
		outcome::result<void> construct();

		u64 size() const;
		outcome::result<std::shared_ptr<abstract_syntax_tree>> parse();
	private:
		outcome::result<void> construct(const filepath& path);

		static outcome::result<void> verify_source_file(
			const filepath& path
		);
	private:
		filepath m_root_compilation_unit_path;
		detail::directed_graph<filepath, token_list> m_graph;
	};
}
