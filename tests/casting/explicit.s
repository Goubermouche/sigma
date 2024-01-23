i32 main() {
	i32* memory = cast<i32*>(malloc(20));
	printf("%d\n", memory);

	ret 0;
}
