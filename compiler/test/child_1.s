i32 WIDTH = 100;
i32 HEIGHT = 30;
i32 MAX_ITER = 1000;

i32 mandelbrot(f64 real, f64 imag) {
    f64 z_real = 0;
    f64 z_imag = 0;
    f64 z_real_sq;
    f64 z_imag_sq;

    for (i32 i = 0; i < MAX_ITER; i++) {
        z_real_sq = z_real * z_real;
        z_imag_sq = z_imag * z_imag;

        if (z_real_sq + z_imag_sq > 4.0) {
            return i; 
        }

        z_imag = 2 * z_real * z_imag + imag;
        z_real = z_real_sq - z_imag_sq + real;
    }

    return MAX_ITER;
}

char get_char(i32 iterations) {
    if (iterations == MAX_ITER) {
        return ' ';
    }

    char* charset = ".,-~:;=!*#$@";
    i32 charset_size = 12;
    return charset[iterations % charset_size];
}