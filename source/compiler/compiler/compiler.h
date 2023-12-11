// Root compiler file. Contains the core implementation of the compiler and the
// related sub-processes. The main invocation point is located in the main.cpp file. 
// The required steps which need to be taken to create a valid compilation together
// with their specific order can be seen below:
//
// AST generation                     Lowering
// +-------+   +--------+   +-----+   +----+   +----------------+   +------------+
// | lexer +-->| parser +-->| AST +-->| IR +-->| code_generator +-->| executable |
// +-------+   +--------+   +-----+   +----+   +----------------+   +------------+
//
// todo: add a semantic analyzer, which would serve as a step before AST traversal,
//       and would handle all error checking the current AST implementation handles

#pragma once
#include <utility/macros.h>

namespace sigma {
	class compiler {
	public:
		static void compile();
	};
} // namespace sigma
