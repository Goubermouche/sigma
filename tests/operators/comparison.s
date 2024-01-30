i32 main() {
	i32 a = 100;
	i32 b = 100;

	printf("%d < %d  = %d\n", a, b, a < b);
	printf("%d > %d  = %d\n", a, b, a > b);
	printf("%d <= %d = %d\n", a, b, a <= b);
	printf("%d >= %d = %d\n", a, b, a >= b);
	printf("%d == %d = %d\n", a, b, a == b);
	printf("%d != %d = %d\n", a, b, a != b);

  	printf("lit: %d\n", 100 > 200);

	i32* x = cast<i32*>(malloc(10));
	i32* y = cast<i32*>(malloc(10));

	printf("ptr: %d\n", x > y);

	ret 0;
}
