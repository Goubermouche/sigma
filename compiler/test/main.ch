// todo: explicit cast (!)
//  - cast<i32>(value)

i32 main() {
	f32 a = 10;
	f32 b = 10;
	bool result = a == b && a != b;
	print("%i\n", result);
}