
#include "source/compiler/compiler.h"

using namespace channel::types;

/**
 * \brief Runs the compiler. Generates an exe from the given file (\a argv[1]) at the given location (\a argv[2])
 * \param argc Argument count
 * \param argv Argument values
 * \return Status code.
 */
i32 main(i32 argc, char* argv[]) {
	channel::console::init();

	// required arguments:
	// executable path (automatic)
	// source file path
	// target executable directory
	if(argc != 3) {
		channel::console::out
			<< channel::color::red
			<< "invalid argument count\n"
			<< channel::color::white;
		return 1;
	}

	// initialize the compiler 
	channel::compiler_settings settings;
	settings.optimization_level = channel::optimization_level::high;
	settings.size_optimization_level = channel::size_optimization_level::high;
	settings.vectorize = true;

	channel::compiler compiler(settings);

	// check for compilation errors
	if(const auto compilation_result = compiler.compile(argv[1], argv[2])) {
		compilation_result.value()->print();
		return 1;
	}

	return 0;
}
