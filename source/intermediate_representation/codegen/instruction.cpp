#include "instruction.h"
#include "intermediate_representation/target/arch/x64/x64.h"

namespace sigma::ir {
	auto instruction::is_terminator() const -> bool {
		return type == TERMINATOR || type == INT3 || type == UD2;
	}
}
