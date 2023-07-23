#include "child_1.s"
// #include "child_2.s"

// todo: do not include C funcs in redefinition checks
// todo: better logging functions 
// todo: better linker error handling

i32 main() {
    f64 x_min = -2.0;
    f64 x_max = 1.0;
    f64 y_min = -1.0;
    f64 y_max = 1.0;
    
    f64 x_step = (x_max - x_min) / WIDTH;
    f64 y_step = (y_max - y_min) / HEIGHT;
    
    for (f64 y = y_min; y < y_max; y = y + y_step) {
        for (f64 x = x_min; x < x_max; x = x + x_step) {
            i32 iterations = mandelbrot(x, y);
            // call to 'get_char(iterations)' causes a linker error to occur
            printc('x');
        }
    
        printc('\n');
    }
    
    return 0;
}