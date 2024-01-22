void test(u8 value) {}

i8 run(i8 value) {
    test(value);
    ret 42;
}

i32 main() {
    printf("test %d %s", run(100), "xd");
    printf("test %d %s", run(100) + 12 + 2, "xd");

    u64 value = 120;
    printf("test %d %s", run(100) + 12 + value, "xd");
    ret 0;
}
