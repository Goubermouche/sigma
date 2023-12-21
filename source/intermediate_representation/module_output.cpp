#include "module_output.h"

namespace sigma::ir {
	void module_output::add_section(const std::string& name, module_section::module_section_flags flags, comdat::comdat_type comdat) {
		sections.push_back({
			.name = std::string(name),
			.flags = flags,
			.com = {.ty = comdat }
		});
	}
} // namespace sigma::ir
