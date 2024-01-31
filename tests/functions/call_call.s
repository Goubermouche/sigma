i32 run(i32 x) { 
    ret x; 
}

u64 test() {
	ret 12;
}

i32 main() {
    printf("%d\n", run(test()));
    ret 0;
}
