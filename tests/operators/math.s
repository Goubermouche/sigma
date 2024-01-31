i32 main() {
    i32 value1 = 100 + 200;
    i32 value2 = 100 - 200;
    i32 value3 = 100 * 200;
    u8  value4 = 32 * 10 - 1; // 63

    i32 precedence = value1 * value2 - value3;

    printf("%d\n", value1);
    printf("%d\n", value2);
    printf("%d\n", value3);
    printf("%d\n", value4);
    printf("%d\n", precedence);
    ret 0;
}
