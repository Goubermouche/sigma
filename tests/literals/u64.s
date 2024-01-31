i32 main() {
	u64 min = 0;
	u64 max = 18446744073709551615;

	printf("%llu %llu\n", min, max);

	u64 overflow = 18446744073709551616;
	u64 underflow = -1;

	printf("%llu %llu\n", overflow, underflow);

	ret 0;
}
