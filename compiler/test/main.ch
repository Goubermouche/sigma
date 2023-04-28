// todo: explicit cast (!)
//  - cast<i32>(value)

// todo: references (!!)
//  - add support for references (&)

// todo: loops (!!!)
//  - add support for loops 
//    - for
//    - while

i32 main() {
	i32* arr = new i32[3];
	arr[1] = 20;
	print("%i\n", arr[1]++);
	print("%i\n", arr[1]+++ 100);
}