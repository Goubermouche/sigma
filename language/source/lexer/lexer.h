#pragma once

#include "../utility/filesystem.h"
#include "token.h"

namespace language {
	class lexer {
	public:
		lexer(const std::string& source_file);
		token get_token() const;
	private:
		std::ifstream m_source;
	};
}