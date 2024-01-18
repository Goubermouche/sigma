// INFO:
// -   linking: link /OUT:a.exe a.obj /LIBPATH:C:\dev\projects\sigma\source\compiler\test\libraries libcmt.lib libvcruntime.lib libucrt.lib

// TODO: 
// -   check for memory oversteps in the parser
// -   add namespaces to error messages, whenever applicable (ie. x::y::test)

namespace x {
	namespace y {
		i32 test(i32 a) { ret 1; }
	}

	i32 test(i32 a) { ret 1; }
}

i32 test(i32 a) { ret 1; }
i32 test(i32 a, i32 b) { ret a + b; }

i32 main() {
	i32 a = 30;
	
	if(true) {
		a = 40;
	}

	printf("%d\n", test(a, 12));
	ret 0;
}
