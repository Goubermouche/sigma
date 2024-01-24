// INFO:
// -   linking: link /OUT:a.exe a.obj /LIBPATH:C:\dev\projects\sigma\source\compiler\test\libraries libcmt.lib libvcruntime.lib libucrt.lib

// KNOWN BUGS
// -   numerical literals throw cast warnings, which shouldnt happen

// TODO: 
// -   check for memory oversteps in the parser
// -   add namespaces to error messages, whenever applicable (ie. x::y::test)
// -   namespace directives should probably be a part of the function signature? 

// -   cleanup ir gen alignment sizes (u64 vs u32 vs u16)

i32 main() {
	printf("%d\n", sizeof(i8));
	printf("%d\n", sizeof(i16));
	printf("%d\n", sizeof(i32));
	printf("%d\n", sizeof(i64));
	printf("%d\n", sizeof(u8));
	printf("%d\n", sizeof(u16));
	printf("%d\n", sizeof(u32));
	printf("%d\n", sizeof(u64));
	printf("%d\n", sizeof(bool));
	printf("%d\n", sizeof(char));
	printf("%d\n", sizeof(i32*));
	printf("%d\n", sizeof(i32**));
    
	ret 0;
}
