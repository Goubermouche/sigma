void run(i32 x) {}

u64 test() {
	ret 12;
}

i32 main() {
    run(test());
    ret 0;
}
