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

i32 main() {
	bool a = true;
	bool b = false;

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
	puts("THIS IS A PUTS TEST");

	ret 0;
}
