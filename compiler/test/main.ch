// todo: imports
// todo: explicit cast
//       cast<i32>(value)
// todo: i32a causes an assertion to be triggered
// todo: check for, and handle, divide by 0 exceptions

u64 g = 100;

f32 func(i32 a, f32 x, i32 b) {
    i32 pog = a;
    return x;
}

i32 main() {
    i32 x = func(1, 2.0f, 3);
    func(1, 2.0f, g);
}