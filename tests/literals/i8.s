i32 main() {
	i8 min = -128;
	i8 max = 127;

	printf("%d %d\n", min, max);

	i8 overflow = 128;
	i8 underflow = -129;

	printf("%d %d\n", overflow, underflow);

	ret 0;
}
