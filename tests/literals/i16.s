i32 main() {
	i16 min = -32768;
	i16 max = 32767;

	printf("%d %d\n", min, max);

	i16 overflow = -32769;
	i16 underflow = 32768;

	printf("%d %d\n", overflow, underflow);

	ret 0;
}
