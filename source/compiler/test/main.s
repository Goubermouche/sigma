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
//     -   the parser returns an empty error in some cases
//     -   set crashes with more than 4(?) parameters
// -   TESTS:
//     -    add more test cases
//     -    figure out why the array3D test case wasnt working
// -   convert ir bool types to sigma types correctly for TB stuff
// -   implicit returns for non-void functions should not be a thing

i32 main() {
    printf("%d\n", true);
    ret 0;
}
