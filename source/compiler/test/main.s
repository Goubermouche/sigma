// INFO:
// -   linking: link /OUT:a.exe a.obj /LIBPATH:C:\dev\projects\sigma\source\compiler\test\libraries libcmt.lib libvcruntime.lib libucrt.lib

// TODO: 
// -   check for memory oversteps in the parser
// -   add namespaces to error messages, whenever applicable (ie. x::y::test)
// -   namespace directives should probably be a part of the function signature? 
i32 main() {
		i32 a = -1;
		u32 a1 = -1;
		u32 ab = -1;
		u32 a2 = -1;
		u32 a3 = -1;
		printf("%lu\n", a);

    ret 0;
}
