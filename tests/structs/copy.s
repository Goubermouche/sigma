struct user {
	i32* k;
	i32 x;
};

i32 main() {
	user usr1;

	usr1.k = cast<i32*>(malloc(8));
	usr1.k[0] = 123;
	usr1.k[1] = 321;
	usr1.x = 111;
	
	user usr2 = usr1;
	
	printf("key: %d %d %d\n", usr1.k[0], usr1.k[1], usr1.x);
	printf("key: %d %d %d\n", usr2.k[0], usr2.k[1], usr2.x);

	ret 0;
}
