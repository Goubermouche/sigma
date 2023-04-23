// todo: lexer (!)
//  - clean up the parsing functions for literals and fix some bugs with parsing escape sequences

// todo: types (!!)
//  - get_base should probably return a pointer if its the type

// todo: explicit cast (!)
//  - cast<i32>(value)

// todo: math (!)
//  - check for, and handle, divide by 0 exceptions

i32 main() {
	bool** matrix = new bool*[2];
	matrix[0] = new bool[2];
	matrix[1] = new bool[2];
	matrix[0][0] = true;
	matrix[1][0] = false;
	matrix[1][1] = true;
	print("values: %i %i %i\n", matrix[0][0], matrix[1][0], matrix[1][1]);
}