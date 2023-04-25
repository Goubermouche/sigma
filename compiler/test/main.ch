// todo: explicit cast (!)
//  - cast<i32>(value)

void test(i32** arg) {
	arg[0][1] = 200;
}

i32 main() {
	// format
	char* fmt = "%i %i %i";

	// array
	i32** values = new i32*[10];
	values[0] = new i32[3];
	values[0][0] = 100;
	values[0][2] = 300;

	// calls 
	test(values);
	print(fmt, values[0][0], values[0][1], values[0][2]);
}