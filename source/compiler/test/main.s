// INFO:
// -   linking: link /OUT:a.exe a.obj /LIBPATH:C:\dev\projects\sigma\source\compiler\test\libraries libcmt.lib libvcruntime.lib libucrt.lib

// KNOWN BUGS
// -   numerical literals throw cast warnings, which shouldnt happen

// TODO: 
// -   check for memory oversteps in the parser
// -   add namespaces to error messages, whenever applicable (ie. x::y::test)
// -   namespace directives should probably be a part of the function signature? 

// -   cleanup ir gen alignment sizes (u64 vs u32 vs u16)

// -   unify parse_identifier_expression and parse_identifier_statement

i32 main() {
	i32* memory = cast<i32*>(malloc(100));
	memory[0] = 2222;
	memory[1] = 3333;
	memory[2] = 4444;

	printf("%d, %d\n", memory[2], memory[1]);

	ret 0;
}
