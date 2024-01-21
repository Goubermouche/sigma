// INFO:
// -   linking: link /OUT:a.exe a.obj /LIBPATH:C:\dev\projects\sigma\source\compiler\test\libraries libcmt.lib libvcruntime.lib libucrt.lib

// TODO: 
// -   check for memory oversteps in the parser
// -   add namespaces to error messages, whenever applicable (ie. x::y::test)
// -   namespace directives should probably be a part of the function signature? 

// -   rework the AST
// -   better retracing in function args
// -   support for binop locations etc (location in create_node)
// -   
// -   

void test(u8 value) {}

i8 run(i8 value) {
    test(value);
    ret 42;
}

i32 main() {
    printf("test %d %s", run(100), "xd");
    ret 0;
}
