// INFO:
// -   linking: link /OUT:a.exe a.obj /LIBPATH:C:\dev\projects\sigma\source\compiler\test\libraries libcmt.lib libvcruntime.lib libucrt.lib

// TODO: 
// -   check for memory oversteps in the parser
// -   add namespaces to error messages, whenever applicable (ie. x::y::test)
// -   namespace directives should probably be a part of the function signature? 
namespace a {
	namespace xxxx {
		i32 test(i32 a, i32 b) { ret a; }
	}
}

i32 main() {
	i32 a = 30;
	
	if(true) {
		a = 40;
	}

	printf("%d\n", a::test(a, 12));
	ret 0;
}
