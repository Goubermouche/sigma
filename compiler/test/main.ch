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

// numerical literal auto upcasts 
//  - handle upcasts for numerical literals 
//  - 2 * double_var etc

i32 main() {
	bool value = false;
	bool test = !value;

	if(test) {
		print("here\n");
	}

	return 0;
}