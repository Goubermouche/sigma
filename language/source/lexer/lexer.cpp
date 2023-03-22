#include "lexer.h"

namespace language {
	lexer::lexer(const std::string& source_file)
		: m_source(std::ifstream(source_file)) {

		if(!std::filesystem::exists(source_file) || m_source.bad()) {
			ASSERT(false, std::string("cannot open source file '" + source_file + "'\n").c_str());
		}
	}

	token lexer::get_token() const
	{
		return token::var_i16;
	}
}