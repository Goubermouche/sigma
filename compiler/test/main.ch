// todo: explicit cast (!)
//  - cast<i32>(value)

// todo: logical expressions: 
//  - upcast for comparisons 

i32 main() {
	i32 integer = 2 * (10 / 5);
	bool boolean = integer < 3 && true;

	print("%i %i\n", integer, boolean);
}