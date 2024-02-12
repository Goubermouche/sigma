namespace x {
	struct user {
		i32* k;
		i32 x;
	};
}

namespace y {
	struct user {
		i32* k;
		i32 x;
	};
}

void print(x::user usr) {
	printf("%d\n", usr.x);
}

i32 main() {
	x::user my_user;
	
	my_user.k = cast<i32*>(malloc(8));
	my_user.k[0] = 123;
	my_user.k[1] = 321;
	my_user.x = 111;

	print(my_user);

	ret 0;
}
