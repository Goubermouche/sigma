// todo: imports
// todo: explicit cast
//       cast<i32>(value)
// todo: i32a causes an assertion to be triggered
// todo: check for, and handle, divide by 0 exceptions

u64 g = 100;

i32 func(f32 a, i32 b) {
    return g * a / b;
}

i32 main() {
    print(func(20, 3)); // 666
}