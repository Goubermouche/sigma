u64 mandelbrot(
    f64 real, 
    f64 imag,
    u64 max_iterations
) {
    f64 z_real = 0;
    f64 z_imag = 0;
    f64 z_real_sq;
    f64 z_imag_sq;

    for (u64 i = 0; i < max_iterations; i++) {
        z_real_sq = z_real * z_real;
        z_imag_sq = z_imag * z_imag;

        if (z_real_sq + z_imag_sq > 4.0) {
            return i; 
        }

        z_imag = 2.0 * z_real * z_imag + imag;
        z_real = z_real_sq - z_imag_sq + real;
    }

    return max_iterations;
}

char get_char(
    u64 iterations,
    u64 max_iterations
) {
    if (iterations == max_iterations) {
        return ' ';
    }

    char* charset = ".,-~:;=!*#$@";
    u64 charset_size = 12;
    return charset[iterations % charset_size];
}