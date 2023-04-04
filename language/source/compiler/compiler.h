#pragma once
#include "../codegen/codegen_visitor.h"
#include "../parser/parser.h"
#include "../utility/timer.h"

namespace channel {
	class compiler {
	public:
		compiler(const std::string& source_file, const std::vector<std::string>& arguments);
	};
}
