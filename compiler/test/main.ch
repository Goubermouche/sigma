// todo: lexer (!)
//  - clean up the parsing functions for literals and fix some bugs with parsing escape sequences

// todo: explicit cast (!)
//  - cast<i32>(value)

i32 main() {
	u64**** matrix = new u64***[2];
	matrix[0] = new u64**[2];
	matrix[0][0] = new u64*[2];
	matrix[0][0][0] = new u64[2];
	matrix[0][0][0][0] = 100u;
	matrix[0][0][0][1] = 200u;

	f32 fval = 127.023f;
	print("values: %u %u %.3f\n", matrix[0][0][0][0], matrix[0][0][0][1], fval);
}