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

// BUGS: 
// i32 main() {
// 	i32 value;
// 	if(true) {
// 		value = 200;
// 	}
// 
// 	printf("value is: %d\n", value);
// 	ret 0;
// }

i32 main() {
	i32 value = 0;

	if(false) {}
	else {
		value = 100;
	}

	printf("value is: %d\n", value);
	ret 0;
}
