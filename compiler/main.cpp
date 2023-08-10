#include "source/compiler/compiler.h"
using namespace sigma::types;

/**
 * \brief Runs the compiler. Generates an exe from the given file (\a argv[1]) at the given location (\a argv[2])
 * \param argc Argument count
 * \param argv Argument values
 * \return Status code.
 */
i32 main(i32 argc, char* argv[]) {
	sigma::console::init();

	// required arguments:
	// executable path (automatic)
	// source file path
	// target executable directory
	// if(argc != 3) {
	// 	sigma::console::out
	// 		<< sigma::color::red
	// 		<< "invalid argument count\n"
	// 		<< sigma::color::white;
	// 	return 1;
	// }

	// initialize the compiler settings
	sigma::compiler_settings settings;
	settings.opt_level = sigma::optimization_level::high;
	settings.size_opt_level = sigma::size_optimization_level::high;
	settings.vectorize = true;

	// construct the compiler object
	sigma::compiler compiler(settings);

	const auto compilation_result = compiler.compile(
		"./test/main.s", 
		"./test/"
	);

	// check for compilation errors
	if(compilation_result.has_error()) {
		compilation_result.get_error()->print();
		return 1;
	}

	return 0;
}
