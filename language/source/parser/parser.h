#pragma once

#include "../lexer/lexer.h"
#include "../codegen/evaluator.h"

namespace channel {
	class parser {
	public: 
		parser(const std::string& source_file);
	private:
		lexer m_lexer;
	};
}