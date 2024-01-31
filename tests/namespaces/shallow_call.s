namespace test {
    i32 test() {
        ret 20;
    }
}

i32 main() {
    printf("%d\n", test::test());
    ret 0;
}