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
//  - numerical literals should always be cast to the desired type, as long as it is not a boolean

void fizz_buzz(u64 iteration_count) {
	for(u64 i = 0; i <= iteration_count; ++i) {
		if(i % 15 == 0) {
			print("FizzBuzz\t");
		}
		else if(i % 3 == 0) {
			print("Fizz\t");
		}
		else if(i % 5 == 0) {
			print("Buzz\t");
		}
		else {
			print("%d\t", i);
		}
	}
}

i32 main() {
	fizz_buzz(100);
}