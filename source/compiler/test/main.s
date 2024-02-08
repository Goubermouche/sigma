// INFO:
// -   linking: link /OUT:a.exe a.obj /LIBPATH:C:\dev\projects\sigma\source\compiler\test\libraries libcmt.lib libvcruntime.lib libucrt.lib

// KNOWN BUGS
// -   numerical literals throw cast warnings, which shouldnt happen
// -   numerical literals are always interpreted as i32

// TODO: 
// -   DIAGNOSTICS:
//     -   better messages
//     -   more info related to numerical errors (hex etc)
//     -   add namespaces to messages, whenever applicable (ie. x::y::test)
// -   TESTS:
//     -    add more test cases

i32 main() {
	i32*** memory = cast<i32***>(malloc(100));

	memory[0] = cast<i32**>(malloc(100));
	memory[1] = cast<i32**>(malloc(100));

	memory[0][0] = cast<i32*>(malloc(100));
	memory[1][0] = cast<i32*>(malloc(100));

	memory[0][0][0] = 1;
	memory[1][0][0] = 2;

	printf("%d %d\n", memory[0][0][0], memory[1][0][0]);
	ret 0;
}
