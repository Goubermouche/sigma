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

	struct s1 {
		i32 a;
	};

	printf("%d\n", sizeof(s1));
	
	struct s2 {
		i32 a;
		char** b;
	};

	printf("%d\n", sizeof(s2));
    
	ret 0;
}

