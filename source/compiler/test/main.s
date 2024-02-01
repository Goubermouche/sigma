// INFO:
// -   linking: link /OUT:a.exe a.obj /LIBPATH:C:\dev\projects\sigma\source\compiler\test\libraries libcmt.lib libvcruntime.lib libucrt.lib

// KNOWN BUGS
// -   numerical literals throw cast warnings, which shouldnt happen
// -   numerical literals are always interpreted as i32

// TODO: 
// -   add namespaces to error messages, whenever applicable (ie. x::y::test)
// -   cleanup ir gen alignment sizes (u64 vs u32 vs u16)
// -   set crashes with more than 4(?) parameters
// -   implicit returns for non-void functions should be a thing

i32 run(i32 x) { 
    ret x; 
}

i32 test() {
	ret 12;
}

i32 main() {
    printf("%d\n", run(test()));
    ret 0;
}

// THIS CRASHES
// i32 main() {
// 	i32* memory = cast<i32*>(malloc(100));
// 	
// 	memory[0] = 1;
// 	memory[1] = 2;
// 	
// 	printf("%d %d\n", memory[0], memory[1]);
// 
// 	ret 0; a
// }