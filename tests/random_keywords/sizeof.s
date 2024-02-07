i32 main() {
	printf("%d\n", sizeof(i8));
	printf("%d\n", sizeof(i16));
	printf("%d\n", sizeof(i32));
	printf("%d\n", sizeof(i64));
	printf("%d\n", sizeof(u8));
	printf("%d\n", sizeof(u16));
	printf("%d\n", sizeof(u32));
	printf("%d\n", sizeof(u64));
	printf("%d\n", sizeof(bool));
	printf("%d\n", sizeof(char));
	printf("%d\n", sizeof(i32*));
	printf("%d\n", sizeof(i32**));

	struct s1 {
		i32 a;
	};

	printf("%d\n", sizeof(s1));
	
	struct s2 {
		i32 a;
		char** b;
	};

	printf("%d\n", sizeof(s2));
    
	ret 0;
}
