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
//  - numerical literals should always be cast to the desired type, as long as it is not a boolean

// declare variables for the number of rows and columns, and a pointer to an array
u64 rows = 20;
u64 cols = 100;
i32* array;

// function for setting the value of a specific pixel in the array
void set_pixel(u64 x, u64 y, i32 value) {
   u64 idx = x + cols * y;
   array[idx] = value;
}

//function for generating the mandelbrot set
void mandelbrot() {
   // set up the initial parameters for the mandelbrot set
   f64 min_re = -2.0;
   f64 max_re = 2.0;
   f64 min_im = -1.2;
   f64 max_im = min_im + (max_re - min_re) * rows / cols;
   f64 re_factor = (max_re - min_re) / (cols - 1);
   f64 im_factor = (max_im - min_im) / (rows - 1);
   u64 max_iterations = 30;

   // iterate through each row and column
   for(u64 y = 0; y < rows; ++y) {
      f64 c_im = max_im + y * im_factor;

      for(u64 x = 0; x < cols; ++x) {
         f64 c_re = min_re + x * re_factor;

         // initialize complex numbers for the mandelbrot set
         f64 z_re = c_re;
         f64 z_im = c_im;
         bool is_inside = true;

         // iterate and check for convergence
         for(u64 n = 0; n < max_iterations; ++n) {
            f64 z_re2 = z_re * z_re;
            f64 z_im2 = z_im * z_im;

            // check for divergence
            if(z_re2 + z_im2 > 4.0) {
               is_inside = false;
               break;
            }

            // calculate the next iteration
            z_im = 2.0 * z_re * z_im + c_im;
            z_re = z_re2 - z_im2 + c_re;
         }

         // set the pixel value if it is inside the Mandelbrot set
         if(is_inside) {
            set_pixel(x, y, 1);
         }
      }
   }
}

// function for printing the contents of the array
void dump() {
   for(u64 idx = 0; idx < rows * cols; idx++) {
      print("%i", array[idx]);

      // print a newline character at the end of each row
      if(((idx + 1) % cols) == 0) {
         print("\n");
      }
   }
}

// main entry point
i32 main() {
   u64 array_size = rows * cols;
   array = new i32[array_size];	

   // initialize the array with zeros
   for(u64 i = 0; i < array_size; i++) {
      array[i] = 0;
   }

   // generate the mandelbrot set and print the result
   mandelbrot();
   dump();

   return 0;
}
