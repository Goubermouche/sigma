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
// -   BUGS:
//     -   STRUCT parameters are buggy af (TODO: proper > operator for types)
//     -   return types are unchecked

struct user {
	i32* k;
	i32 x;
};

void print(user usr) {
	printf("key: %d %d %d\n", usr.k[0], usr.k[1], usr.x);
}

i32 main() {
	user my_user;

	my_user.k = cast<i32*>(malloc(8));
	my_user.k[0] = 123;
	my_user.k[1] = 321;
	my_user.x = 111;
	
	printf("key: %d %d %d\n", my_user.k[0], my_user.k[1], my_user.x);
	print(my_user);

	ret 0;
}
