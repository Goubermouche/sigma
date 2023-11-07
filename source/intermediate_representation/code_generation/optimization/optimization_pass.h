#pragma once
#include "intermediate_representation/code_generation/code_generator_context.h"

namespace ir::cg {
	class optimization_pass {
	public:
		virtual void apply(code_generator_context& context) = 0;
	};

	// TODO: we'll probably need to add a priority to each pass and sort
	//       them before applying them

	class optimization_pass_list {
	public:
	public:
		optimization_pass_list(const std::vector<s_ptr<optimization_pass>>& passes);

		void apply(code_generator_context& context);
	private:
		std::vector<s_ptr<optimization_pass>> m_passes;
	};
}
