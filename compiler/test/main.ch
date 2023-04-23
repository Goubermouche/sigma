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
	i32** matrix = new i32*[2];
	matrix[0] = new i32[2];
	matrix[1] = new i32[2];
	matrix[0][0] = 1;
	matrix[1][0] = 2;
	matrix[1][1] = 3;
	print("values: %i %i %i\n", matrix[0][0], matrix[1][0], matrix[1][1]);
}