// todo: project cleanup

// todo: function args
// todo: imports
// todo: explicit cast
//       cast<i32>(value)
// todo: check for, and handle, divide by 0 exceptions

f32 f = 100.0f;
u64 v = 20 * f;

f32 func() {
   return v * 20.0f;
}

i32 main() {
    u32 x = 20;
    i8 c = func() * 20u;
    u32 y;
    y = func() / 100.0f;
}