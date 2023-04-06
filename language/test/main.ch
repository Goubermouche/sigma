i32 g = 10;

i32 func() {
    return 20;
}

i32 main() {
    i32 a = 3 * func() + g;
    return a;
}