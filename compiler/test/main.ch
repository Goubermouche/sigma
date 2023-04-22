// todo: lexer (!)
//  - clean up the parsing functions for literals and fix some bugs with parsing escape sequences

// todo: arrays (!!!)
//  - add support for multi-bracket array invocations
//  - array[0][0][0] ...

// todo: types (!!)
//  - get_base should probably return a pointer if its the type

// todo: explicit cast (!)
//  - cast<i32>(value)

// todo: math (!)
//  - check for, and handle, divide by 0 exceptions

i32 main() {
	char** strings = new char*[3];
	strings[0] = "Hello, world! %i";
	bool working = true;
	print(strings[0], working);
}