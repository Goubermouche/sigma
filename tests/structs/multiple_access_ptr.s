i32 main() {
	struct key {
		i32* value;
	};

	struct user {
		key k;
	};

	user my_user;

	my_user.k.value = cast<i32*>(malloc(sizeof(i32) * 2));
	my_user.k.value[0] = 123;
	my_user.k.value[1] = 321;

	printf("key: %d %d\n", my_user.k.value[0], my_user.k.value[1]);
	ret 0;
}
