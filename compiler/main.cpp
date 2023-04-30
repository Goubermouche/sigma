#include "source/compiler/compiler.h"

using namespace channel::types;

int main(int argc, char* argv[]) {
	_setmode(_fileno(stdout), _O_U16TEXT);

	// use the last argument for now
	channel::compiler::compile("test/main.ch");
	return 0;
}