i32 main() {
	i32*** memory = cast<i32***>(malloc(100));

	memory[0] = cast<i32**>(malloc(100));
	memory[1] = cast<i32**>(malloc(100));

	memory[0][0] = cast<i32*>(malloc(100));
	memory[1][0] = cast<i32*>(malloc(100));

	memory[0][0][0] = 1;
	memory[1][0][0] = 2;

	printf("%d %d\n", memory[0][0][0], memory[1][0][0]);
	ret 0;
}
