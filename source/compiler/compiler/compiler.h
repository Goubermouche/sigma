// Root compiler file. Contains the core implementation of the compiler and the related
// sub-processes. The main invocation point is located in the main.cpp file. The required
// steps which need to be taken to create a valid compilation together with their specific
// order can be seen below:
//
// AST generation                           Lowering (IR)      Lowering (Machine bytecode)
// +-------+  +--------+  +--------------+  +---------------+  +----------------+  +------------+
// | lexer +->| parser +->+ type_checker +->+ IR_translator +->| code_generator +->| executable |
// +-------+  +--------+  +--------------+  +---------------+  +----------------+  +------------+

#pragma once
#include <utility/macros.h>

namespace sigma {
	class compiler {
	public:
		static void compile();
	};
} // namespace sigma
