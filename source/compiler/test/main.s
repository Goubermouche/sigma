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

i32 main() {
	i32 value = 10;

	if(value) {
		printf("yippee\n");
	}

  ret 0;
}
