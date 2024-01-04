i32 test(i32 a, i32 b) {
    ret 0;
}

i32 test(i32 a) {
    ret 0;
}

i32 main() {
    test(1);
    test(1, 2);
    ret 0;
}