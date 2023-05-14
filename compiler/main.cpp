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

	// account for the first argument being the path to the executable
	if(argc != 3) {
		channel::console::out << channel::color::red << "invalid argument count\n" << channel::color::white;
		return EINVAL;
	}

	// initialize the compiler 
	channel::compiler_description description;
	description.root_source_file = argv[1];
	description.executable_location = argv[2];
	description.optimization_level = channel::optimization_level::high;
	description.size_optimization_level = channel::size_optimization_level::high;
	description.vectorize = true;

	channel::compiler compiler(description);

	// check for compilation errors
	if(const auto compilation_result = compiler.compile()) {
		compilation_result->print();
	}

	return 0;
}