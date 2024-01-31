i32 main() {
	i64 min = -9223372036854775808;
	i64 max = 9223372036854775807;

	printf("%lld %lld\n", min, max);

	i64 overflow = -9223372036854775808;
	i64 underflow = 9223372036854775807;

	printf("%lld %lld\n", overflow, underflow);

	ret 0;
}
