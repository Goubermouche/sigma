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
// -   Fix all errors that DEBUG catches, run with fsanitize=address,undefined

i32 main() {
	if(true) {
		printf("true\n");
	}
	else {
		printf("false\n");
	}

	ret 0;
}
