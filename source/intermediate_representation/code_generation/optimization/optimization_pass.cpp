#include "optimization_pass.h"

namespace ir::cg {
	optimization_pass_list::optimization_pass_list(
		const std::vector<s_ptr<optimization_pass>>& passes
	) : m_passes(passes) {}

	void optimization_pass_list::apply(code_generator_context& context) {
		for (const s_ptr<optimization_pass> pass : m_passes) {
			pass->apply(context);
		}
	}
}
