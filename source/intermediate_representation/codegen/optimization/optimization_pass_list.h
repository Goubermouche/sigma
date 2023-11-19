#pragma once
#include "intermediate_representation/codegen/transformation/transformation_context.h"

namespace ir {
	class optimization_pass {
	public:
		virtual void apply(transformation_context& context) = 0;
		virtual ~optimization_pass() = default;
	};

	// TODO: we'll probably need to add a priority to each pass and sort
	//       them before applying them

	class optimization_pass_list {
	public:
		optimization_pass_list(const std::vector<s_ptr<optimization_pass>>& passes);

		void apply(transformation_context& context) const;
	private:
		std::vector<s_ptr<optimization_pass>> m_passes;
	};
}
