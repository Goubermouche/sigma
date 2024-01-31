i32 main() {
	i32 min = -2147483648;
	i32 max = 2147483647;

	printf("%d %d\n", min, max);

	i32 overflow = -2147483649;
	i32 underflow = 2147483648;

	printf("%d %d\n", overflow, underflow);

	ret 0;
}
