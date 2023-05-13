// todo: explicit cast (!)
//  - cast<i32>(value)

// todo: references (!!)
//  - add support for references (&)

// todo: general
//  - continue (!!)
//  - delete[] (!!)
//  - delete (!!)
//  - 'new' for single elements (!!)

// todo: major cleanup (!!!)
//  - documentation
//  - tests

// numerical literal auto upcasts 
//  - handle upcasts for numerical literals 
//  - 2 * double_var etc

i32 width = 100;
i32 height = 30;
i32 max_iter = 1000;

i32 mandelbrot(f64 real, f64 imag) {
    f64 z_real = 0;
    f64 z_imag = 0;
    f64 z_real_sq;
    f64 z_imag_sq;

    for (i32 i = 0; i < max_iter; i++) {
        z_real_sq = z_real * z_real;
        z_imag_sq = z_imag * z_imag;

        if (z_real_sq + z_imag_sq > 4.0) {
            return i; 
        }

        z_imag = 2 * z_real * z_imag + imag;
        z_real = z_real_sq - z_imag_sq + real;
    }

    return max_iter;
}

char get_char(i32 iterations) {
    if (iterations == max_iter) {
        return ' ';
    }

    char* charset = ".,-~:;=!*#$@";
    i32 charset_size = 12;
    return charset[iterations % charset_size];
}

i32 main() {
    f64 x_min = -2.0;
    f64 x_max = 1.0;
    f64 y_min = -1.0;
    f64 y_max = 1.0;
    
    f64 x_step = (x_max - x_min) / width;
    f64 y_step = (y_max - y_min) / height;
    
    for (f64 y = y_min; y < y_max; y += y_step) {
        for (f64 x = x_min; x < x_max; x += x_step) {
            i32 iterations = mandelbrot(x, y);
            printc(get_char(iterations));
        }
    
        printc('\n');
    }
    
    return 0;
}