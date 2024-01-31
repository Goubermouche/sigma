i32 main() {
	u32 min = 0;
	u32 max = 4294967295;

	printf("%u %u\n", min, max);

	u32 overflow = 4294967296;
	u32 underflow = -1;

	printf("%u %u\n", overflow, underflow);

	ret 0;
}
