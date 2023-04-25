// todo: lexer rework (!!!)
//  - tokenize everything first 
//    - check for tokenizer errors first
//  - store tokens in a vector together with the current value

// todo: explicit cast (!)
//  - cast<i32>(value)

// void test(i32** arg) {
// 	arg[0][1] = 200;
// }

i32 main() {
	// i32** values = new i32*[10];
	// values[0] = new i32[3];
	// values[0][0] = 100;
	// values[0][2] = 300;
	// 
	// char c 
	// char* fmt = "%i %i %i %.3f\n";
	// 
	// f32 fval = 1024.0f;
	// 
	// test(values);
	// print(fmt, values[0][0], values[0][1], values[0][2], fval);
	char a = 'a';
	char b = 'b';
	char c = 'c';

	char* str = new char[3];
	str[0] = a;
	str[1] = b;
	str[2] = c;
	print(str);
}