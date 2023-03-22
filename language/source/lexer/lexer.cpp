#include "lexer.h"

namespace language {
   

	lexer::lexer(const std::string& source_file)
		: m_source(std::ifstream(source_file)) {

		if(!std::filesystem::exists(source_file) || m_source.bad()) {
			ASSERT(false, std::string("cannot open source file '" + source_file + "'\n").c_str());
		}
	}

	// considerations: handle nested scopes using a stack (when we encounter
	//                 a new l_brace we push a new scope to the stack, then
	//                 we add variables to that scope, once we encounter an
	//                 r_brace we pop the stack.

	// implications:   add a scope class which will contain all variables in
	//                 the given scope. note that we need to add a check for
	//                 l_brace/r_brace into the parser main loop, and manage
	//                 the scope stack over there.

    token lexer::get_token() {
        return token::end_of_file;
    }
}