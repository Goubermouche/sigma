i32 main() {
	i32* memory = cast<i32*>(malloc(100));
	
	memory[0] = 1;
	memory[1] = 2;

	printf("%d %d\n", memory[0], memory[1]);

	ret 0;
}