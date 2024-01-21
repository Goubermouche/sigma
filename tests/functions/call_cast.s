void test(u8 value) {}

i8 run(i8 value) {
    test(value);
    ret 42;
}

i32 main() {
    printf("test %d %s", run(100), "xd");
    ret 0;
}
