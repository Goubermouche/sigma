#include "source/compiler/compiler.h"

using namespace channel::types;

int main(int argc, char* argv[]) {
	// use the last argument for now
	channel::compiler::compile("test/main.ch");

	return 0;
}