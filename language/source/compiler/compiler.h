#pragma once
#include "../utility/timer.h"

namespace channel {
	class compiler {
	public:
		compiler(const std::string& source_file, const std::vector<std::string>& arguments);
	};
}
