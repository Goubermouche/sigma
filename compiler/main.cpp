#include "source/compiler/compiler.h"

using namespace channel::types;

i32 main(/*int argc, char* argv[]*/) {
	channel::console::init();

	// use the last argument for now
	channel::compiler::compile("test/main.ch");
	return 0;
}