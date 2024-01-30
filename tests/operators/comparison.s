i32 main() {
	i32 a = 100;
	i32 b = 100;

	printf("%d < %d  = %d\n", a, b, a < b);
	printf("%d > %d  = %d\n", a, b, a > b);
	printf("%d <= %d = %d\n", a, b, a <= b);
	printf("%d >= %d = %d\n", a, b, a >= b);
	printf("%d == %d = %d\n", a, b, a == b);
	printf("%d != %d = %d\n", a, b, a != b);

    printf("%d\n", 100 > 200);

	ret 0;
}