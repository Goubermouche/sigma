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
//     -   type parameters do not take namespaces into account (just identifiers/base/ptr)
//     -   return types are unchecked

i32 main() {
	i32* x;

	ret x;
}
