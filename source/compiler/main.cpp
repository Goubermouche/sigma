#include "compiler/compiler.h"

using namespace utility::types;

auto main(i32 argc, char* argv[]) -> i32 {
	SUPPRESS_C4100(argc);

	const sigma::compiler_description description {
		.path = argv[1],
		.target = { sigma::ir::arch::X64, sigma::ir::system::WINDOWS }
	};

	// compile the specified description, check for errors after we finish
	PRINT_ERROR(sigma::compiler::compile(description));
  return 0;
}
