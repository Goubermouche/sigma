// INFO:
// -   linking: link /OUT:a.exe a.obj /LIBPATH:C:\dev\projects\sigma\source\compiler\test\libraries libcmt.lib libvcruntime.lib libucrt.lib

// TODO: 
// -   function registry
// -   variable registry
// -   stack scope system
// -   typechecker
//     -   convert values into IR types
// -   parser
//     -   check for memory oversteps
// -   IR translator 
//     -   cleanup
//     -   dynamic programming impl? 

i32 main() {
	bool a = true;
	bool b = true;

	if(a) {
		if(b) {
			printf("A && B\n");
		}
		else {
			printf("A\n");
		}
	}
	else if(b) {
		printf("B\n");
	}
	else {
		printf("fallback\n");
	}

	printf(":D\n");

	ret 0;
}
