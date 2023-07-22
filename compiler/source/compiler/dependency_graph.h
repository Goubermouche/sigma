#pragma once
#include "compilation_unit.h"
#include "utility/thread_pool.h"
#include "utility/containers/directed_graph.h"

namespace sigma {
	class dependency_graph {
	public:
		dependency_graph(const filepath& root_path);

		void print() const;
		outcome::result<void> verify() const;
		outcome::result<void> traverse_compile(detail::thread_pool& pool);
		outcome::result<void> construct();

		std::shared_ptr<code_generator_context> get_context() const;
		u64 size() const;
	private:
		outcome::result<void> construct(const filepath& path);
	private:
		filepath m_root_compilation_unit_path;
		detail::directed_graph<filepath, compilation_unit> m_graph;
	};
}
