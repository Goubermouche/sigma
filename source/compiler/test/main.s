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
//     -    figure out why the array3D test case wasnt working
// -   convert ir bool types to sigma types correctly for TB stuff
// -   implicit returns for non-void functions should not be a thing


i32 main() {
	printf("%d %d %d %d %d %d %d\n", 1, 2, 3, 4, 5, 6, 7);
	ret 0;
}


