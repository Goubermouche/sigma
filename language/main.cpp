#include "source/compiler/compiler.h"

using namespace channel::types;

int main(int argc, char* argv[]) {
	// use the last argument for now
	channel::compiler::compile(argv[argc - 1]);
	return 0;
}