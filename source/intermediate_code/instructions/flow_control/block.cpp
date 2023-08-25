#include "block.h"

namespace ir {
	block::block(const std::string& name)
		: value(name) {}

	std::string block::to_string() {
		return m_name + ":";
	}

	const std::vector<instruction_ptr>& block::get_instructions() const {
		return m_instructions;
	}
}