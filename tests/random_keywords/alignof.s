i32 main() {
	printf("%d\n", alignof(i8));
	printf("%d\n", alignof(i16));
	printf("%d\n", alignof(i32));
	printf("%d\n", alignof(i64));
	printf("%d\n", alignof(u8));
	printf("%d\n", alignof(u16));
	printf("%d\n", alignof(u32));
	printf("%d\n", alignof(u64));
	printf("%d\n", alignof(bool));
	printf("%d\n", alignof(char));
	printf("%d\n", alignof(i32*));
	printf("%d\n", alignof(i32**));

	struct s1 {
		i32 a;
	};

	printf("%d\n", alignof(s1));
	
	struct s2 {
		i32 a;
		char** b;
	};

	printf("%d\n", alignof(s2));
    
	ret 0;
}
