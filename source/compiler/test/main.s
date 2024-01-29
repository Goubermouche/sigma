// INFO:
// -   linking: link /OUT:a.exe a.obj /LIBPATH:C:\dev\projects\sigma\source\compiler\test\libraries libcmt.lib libvcruntime.lib libucrt.lib

// KNOWN BUGS
// -   numerical literals throw cast warnings, which shouldnt happen

// TODO: 
// -   add namespaces to error messages, whenever applicable (ie. x::y::test)
// -   cleanup ir gen alignment sizes (u64 vs u32 vs u16)
// -   conjunction & disjunction operators

// CHECK:
// -   bool b = 100 > 200;

i32 main() {
	i32 a = 10;
	u64 b = 20;

	bool res = a == b;
	ret 0;
}
