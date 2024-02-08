i32 main() {
	struct key {
		i32 value;
	};

	struct user {
		key k;
	};

	user my_user;

	my_user.k.value = 100;

	printf("key: %d\n", my_user.k.value);
	ret 0;
}
