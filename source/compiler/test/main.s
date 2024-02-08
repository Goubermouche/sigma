// INFO:
// -   linking: link /OUT:a.exe a.obj /LIBPATH:C:\dev\projects\sigma\source\compiler\test\libraries libcmt.lib libvcruntime.lib libucrt.lib

// KNOWN BUGS
// -   numerical literals throw cast warnings, which shouldnt happen
// -   numerical literals are always interpreted as i32

// TODO: 
// -   DIAGNOSTICS:
//     -   better messages
//     -   more info related to numerical errors (hex etc)
//     -   add namespaces to messages, whenever applicable (ie. x::y::test)
// -   TESTS:
//     -    add more test cases

// 

i32 main() {
	struct vec2 {
		i32 x;
		i32 y;
	};

	vec2 instance;

	instance.x = 10;
	instance.y = 20;

	printf("x: %d\n", instance.x);
	printf("y: %d\n", instance.y);

	ret 0;
}
