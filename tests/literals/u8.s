i32 main() {
	u8 min = 0;
	u8 max = 255;

	printf("%u %u\n", min, max);

	u8 overflow = 256;
	u8 underflow = -1;

	printf("%u %u\n", overflow, underflow);

	ret 0;
}
