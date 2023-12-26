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
  printf("candidate 1\n");
	ret 0;
}

i32 test(i32 a, i32 b) {
	printf("candidate 2\n");
	ret 0;
}

i32 test(i8 a, i16 b) {
	printf("candidate 3\n");
	ret 0;
}

i32 test(i32 a, i16 b) {
	printf("candidate 4\n");
	ret 0;
}

i32 main() {
	i32 value = 100;
	test(value, 1);
	ret 0;
}
