// INFO:
// -   linking: link /OUT:a.exe a.obj /LIBPATH:C:\dev\projects\sigma\source\compiler\test\libraries libcmt.lib libvcruntime.lib libucrt.lib

// TODO: 
// -   parser
//     -   check for memory oversteps
// -   IR translator 
//     -   dynamic programming impl?

i32 test(i32 a) { ret 1; }
i32 test(i32 a, i32 b) { ret 2; }

i32 main() {
	printf("%d\n", test(120, 12));
	ret 0;
}
