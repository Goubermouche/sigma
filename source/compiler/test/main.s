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

i32 deep(i32 x) {
	ret x * 2;
}

i32 test(i32 x, i32 y) {
	ret deep(x) + y;
}

i32 main() {
	i32 val = test(100, 20);
	printf("val : %d\n", val);
	ret 0;
}
