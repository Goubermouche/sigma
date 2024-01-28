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
	i32** memory = cast<i32**>(malloc(100));

	memory[0] = cast<i32*>(malloc(100));
	memory[1] = cast<i32*>(malloc(100));

	memory[0, 0] = 100;
	memory[1, 0] = 200;

	printf("%d\n", memory[1, 0]);
	printf("%d\n", memory[0, 0]);
	printf("%d %d\n", memory[0, 0], memory[1, 0]);
	ret 0;
}

