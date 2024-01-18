namespace test {
    i32 test() {
        ret 20;
    }
}

i32 main() {
    test::test();
    ret 0;
}