// todo: project cleanup
// todo: explicit cast
//       cast<i32>(value)

f32 f = 100.0f;
u64 v = 20 * f;

f32 func() {
    return v;
} 

i32 main() {
    u32 x = 20;
    i8 c;
    c = func() * 20u;
}