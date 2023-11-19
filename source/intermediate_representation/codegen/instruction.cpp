#include "instruction.h"
#include "intermediate_representation/codegen/architectures/x64/x64.h"

namespace ir {
	auto instruction::is_terminator() const -> bool {
		return ty == terminator || ty == INT3 || ty == UD2;
	}
}
