// todo: lexer (!)
//  - clean up the parsing functions for literals and fix some bugs with parsing escape sequences

// todo: explicit cast (!)
//  - cast<i32>(value)

// todo: math (!)
//  - check for, and handle, divide by 0 exceptions

i32 main() {
	u64**** matrix = new u64***[2];
	matrix[0] = new u64**[2];
	matrix[0][0] = new u64*[2];
	matrix[0][0][0] = new u64[2];
	matrix[0][0][0][0] = 100u;
	matrix[0][0][0][1] = 200u;

	matrix[1] = new u64**[2];
	matrix[1][0] = new u64*[2];
	matrix[1][0][0] = new u64[2];
	matrix[1][0][0][0] = 300u;
	matrix[1][0][0][1] = 400u;

	print("values: %u %u %u %u\n", matrix[0][0][0][0], matrix[0][0][0][1], matrix[1][0][0][0], matrix[1][0][0][1]);
}