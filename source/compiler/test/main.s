// INFO:
// -   linking: link /OUT:a.exe a.obj /LIBPATH:C:\dev\projects\sigma\source\compiler\test\libraries libcmt.lib libvcruntime.lib libucrt.lib

// TODO: 
// -   check for memory oversteps in the parser
// -   add namespaces to error messages, whenever applicable (ie. x::y::test)
// -   namespace directives should probably be a part of the function signature? 
// -   handle literal overflow in the typechecker, instead of the IR translator

i32 main() {
	u64 size = 20;
	i32* value = malloc(size);
	printf("%d\n", value);
	void x;

  ret 0;
}
