#include "basic_code_generator.h"

#include "code_generator/abstract_syntax_tree/keywords/file_include_node.h"

namespace channel {
	expected_value basic_code_generator::visit_file_include_node(
		file_include_node& node, 
		const code_generation_context& context
	) {
		return std::make_shared<value>(
			"__temp",
			type(type::base::empty, 0),
			nullptr
		);
	}
}