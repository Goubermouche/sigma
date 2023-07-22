#include "child.s"

// todo: code generator does not need to generate its own context anymore, a new context should be passed
// todo: verify generated IR
// todo: verify main entry point at final module

u64 global = 2;

i32 main() {
	print("hello! %i", value + global);
	return 0;
}