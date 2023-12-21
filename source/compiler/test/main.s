// INFO:
// -   linking: link /OUT:a.exe a.obj /LIBPATH:C:\dev\projects\sigma\source\compiler\test\libraries libcmt.lib libvcruntime.lib libucrt.lib

// TODO: 
// -   stack scope system
// -   typechecker
//     -   convert values into IR types ?
// -   parser
//     -   check for memory oversteps
// -   IR translator 
//     -   dynamic programming impl? 

i32 test(i32 a) {
	ret 0;
}

i32 main() {
	bool value = true;

	if(value) {
		printf("true\n");
	}
	else {
		printf("false\n");
	}

	puts("Hello, world!\n");

	ret 0;
}