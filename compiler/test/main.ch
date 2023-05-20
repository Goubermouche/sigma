// todo: explicit cast (!)
//  - cast<i32>(value)

// todo: references (!!)
//  - add support for references (&)

// todo: general
//  - continue (!!)
//  - delete[] (!!)
//  - delete (!!)
//  - 'new' for single elements (!!)

// todo: major cleanup (!!!)
//  - documentation
//  - tests
// error system truncates to error_message - broken position info for errors

// numerical literal auto upcasts 
//  - handle upcasts for numerical literals 
//  - 2 * double_var etc

i32 main() {
	i32 a = 5;
	i32 b = 3;

	i32 xor_result = a ^ b;

	print("5 XOR 3 results in: %d\n", xor_result);

	return 0;
}