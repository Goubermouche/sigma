i32 pack_bits(u8 a, u8 b, u8 c, u8 d) {
    i32 packed =  a | (b << 8) | (c << 16) | (d << 24);
    return packed;
}

i32 main() {
    i32 value = 10;
    i32* value_ptr = &value;
    *value_ptr = pack_bits(10, 20, 30, 40);
    print("%d\n", value);
}