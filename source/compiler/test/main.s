// INFO:
// -   linking: link /OUT:a.exe a.obj /LIBPATH:C:\dev\projects\sigma\source\compiler\test\libraries libcmt.lib libvcruntime.lib libucrt.lib

// TODO: 
// -   parser
//     -   check for memory oversteps
// -   IR translator 
//     -   dynamic programming impl?

i32 test() {
	printf("a\n");
	ret 0;
}

i32 main() {
	printf("b\n");
	test();
	ret 0;
}
