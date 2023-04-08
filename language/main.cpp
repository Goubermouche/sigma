#include "source/compiler/compiler.h"

using namespace channel::types;

int main() {
	channel::compiler("test/main.ch", {});
	return 0;
}