i32 main() {
    i32 value1 = 100 + 200;
    i32 value2 = 100 - 200;
    i32 value3 = 100 * 200;

    i32 precedence = value1 * value2 - value3;
    ret 0;
}