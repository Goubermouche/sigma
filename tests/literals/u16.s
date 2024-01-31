i32 main() {
	u16 min = 0;
	u16 max = 65535;

	printf("%u %u\n", min, max);

	u16 overflow = 65536;
	u16 underflow = -1;

	printf("%u %u\n", overflow, underflow);

	ret 0;
}
