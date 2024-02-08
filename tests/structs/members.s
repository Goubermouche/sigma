i32 main() {
	struct vec2 {
		i32 x;
		i32 y;
		i32** arr;
	};

	vec2 instance;

	instance.x = 10;
	instance.y = 20;

	instance.arr = cast<i32**>(malloc(16));
	instance.arr[0] = cast<i32*>(malloc(4));
	instance.arr[1] = cast<i32*>(malloc(4));
	instance.arr[0][0] = 1;
	instance.arr[1][0] = 2;

	printf("x: %d\n", instance.x);
	printf("y: %d\n", instance.y);
	printf("arr: %d %d\n", instance.arr[0][0], instance.arr[1][0]);

	ret 0;
}
