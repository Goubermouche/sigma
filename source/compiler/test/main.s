// INFO:
// -   linking: link /OUT:a.exe a.obj /LIBPATH:C:\dev\projects\sigma\source\compiler\test\libraries libcmt.lib libvcruntime.lib libucrt.lib

// TODO: 
// -   check for memory oversteps in the parser
// -   add namespaces to error messages, whenever applicable (ie. x::y::test)
// -   namespace directives should probably be a part of the function signature? 

// -   rework the AST
// -   explicit casting ("cast<type>")

i32 main() {
	if(false) {
		printf("1\n");
	}
  else if(true) {
		printf("2\n");
  }
	else {
		printf("3\n");
	}

	ret 0;
}
