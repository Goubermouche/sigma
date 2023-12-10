#include "optimization_pass_list.h"

namespace sigma::ir {
	optimization_pass_list::optimization_pass_list(
		const std::vector<s_ptr<optimization_pass>>& passes
	) : m_passes(passes) {}

	void optimization_pass_list::apply(transformation_context& context) const {
		for (const s_ptr<optimization_pass>& pass : m_passes) {
			pass->apply(context);
		}
	}
}
