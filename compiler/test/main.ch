// todo: pointers
//  - add support for nested arrays (multiple aserisks)
//  - accessor operators ([index])
//  - cleanup declares
//  - void* doesn't behave as expected

// todo: explicit cast
//       cast<i32>(value)
// todo: i32a causes an assertion to be triggered
// todo: check for, and handle, divide by 0 exceptions

i32 main() {
    i32 count = 10;
	f64* value_1 = new f64[count];
	print(count);
}