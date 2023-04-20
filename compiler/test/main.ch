// todo: pointers
//  - add support for nested arrays (multiple aserisks)

// todo: lexer
//  - clean up the parsing functions for literals 

// todo: explicit cast
//       cast<i32>(value)
// todo: check for, and handle, divide by 0 exceptions

i32 main() {
	u32 uv = -1;
	f32 fv = 29.8f;
	char* text = "pog";
	print("Hello, world! %u %.3f %s\n", uv, fv, text);
}