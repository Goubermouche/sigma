#include "mandelbrot.s"

u64 g_width = 150;
u64 g_height = 60;
u64 g_max_iterations = 1000;

i32 main() {
    f64 x_min = -2.0;
    f64 x_max = 1.0;
    f64 y_min = -1.0;
    f64 y_max = 1.0;
    
    f64 x_step = (x_max - x_min) / g_width;
    f64 y_step = (y_max - y_min) / g_height;
    
    for (f64 y = y_min; y < y_max; y = y + y_step) {
        for (f64 x = x_min; x < x_max; x = x + x_step) {
            u64 iterations = mandelbrot(x, y, g_max_iterations);
            printc(get_char(iterations, g_max_iterations));
        }
    
        printc('\n');
    }
    
    return 0;
}