// INFO:
// -   linking: link /OUT:a.exe a.obj /LIBPATH:C:\dev\projects\sigma\source\compiler\test\libraries libcmt.lib libvcruntime.lib libucrt.lib

// KNOWN BUGS
// -   numerical literals throw cast warnings, which shouldnt happen
// -   numerical literals are always interpreted as i32

// TODO: 
// -   global struct declarations
// -   DIAGNOSTICS:
//     -   better messages
//     -   more info related to numerical errors (hex etc)
//     -   add namespaces to messages, whenever applicable (ie. x::y::test)
// -   BUGS:
//     -   structs as function parameters (rework the < op)

struct key {
	i32* value;
};

i32 main() {
	struct user {
		key k;
	};

	user my_user;

	my_user.k.value = cast<i32*>(malloc(sizeof(i32) * 2));
	my_user.k.value[0] = 123;
	my_user.k.value[1] = 321;
	
	printf("key: %d %d\n", my_user.k.value[0], my_user.k.value[1]);

	ret 0;
}
