## Channel language

## Getting up and running
### Windows 10
<ins>**1. Downloading the repository:**</ins>   
Clone the repository with `git clone https://github.com/Goubermouche/channel-language.git`.

<ins>**2. Building LLVM:**</ins>   
In order to build the project you will need to build [LLVM](https://github.com/llvm/llvm-project/tree/release/16.x) from source. After downloading the [repository](https://github.com/llvm/llvm-project/tree/release/16.x) navigate to the installed directory and create a new out-of-source directory, and navigate into it. Open CMD and run the following commands: 
```cmd
cmake -G "Visual Studio 17 2022" -A x64 -B build
cmake --build build --config Release
```
Note that the process of building LLVM may take around 30 minutes. After building, don't forget to export the path to the built folder to a global variable using the following command:
```cmd
setx LLVM_ROOT path/to/llvm
```

<ins>**3. Project configuration**</ins>   
After building LLVM you'll have to link the necessary libraries and create the basic project structure. To do this, navigate to the root channel directory and run the [setup.bat](https://github.com/Goubermouche/channel-language/blob/1546a311f1e7280321ca556ac2f5c380e62bd97e/setup.bat) script. This will generate the necessary project files for VS 2022, note that if you want to build for something else you'll need to modify the [setup.bat](https://github.com/Goubermouche/channel-language/blob/1546a311f1e7280321ca556ac2f5c380e62bd97e/setup.bat) script. 

## Language example
Below is a simple example for visualizing a slizce of a [Mandelbrot set](https://en.wikipedia.org/wiki/Mandelbrot_set) using the Channel language, the implementation is inspired by a [third party repository](https://github.com/rkosegi/mandelbrot_console).
```cpp
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
```
The expected output, after running the generated .exe, looks like this: 
```
0000000000000000000000000000000000111111111111111111111111000000000000000000000000000000000000000000
0000000000000000000000000000000011111111111111111111111111110000000000000000000000000000000000000000
0000000000000000000000000000000001111111111111111111111111110000000000000000000000000000000000000000
0000000000000000000001000101000011111111111111111111111111100000000000000000000000000000000000000000
0000000000000000000001111111100011111111111111111111111111110000000000000000000000000000000000000000
0000000000000000000011111111110011111111111111111111111111100000000000000000000000000000000000000000
0000000000000000000011111111111111111111111111111111111111110000000000000000000000000000000000000000
0000000000000000000111111111111111111111111111111111111111100000000000000000000000000000000000000000
0000000000000000110111111111111111111111111111111111111111000000000000000000000000000000000000000000
0000000000000001111111111111111111111111111111111111111111000000000000000000000000000000000000000000
0000000000000001111111111111111111111111111111111111111111000000000000000000000000000000000000000000
0000000000000000110111111111111111111111111111111111111111000000000000000000000000000000000000000000
0000000000000000000111111111111111111111111111111111111111100000000000000000000000000000000000000000
0000000000000000000011111111111111111111111111111111111111110000000000000000000000000000000000000000
0000000000000000000011111111110011111111111111111111111111100000000000000000000000000000000000000000
0000000000000000000001111111100011111111111111111111111111110000000000000000000000000000000000000000
0000000000000000000001000101000011111111111111111111111111100000000000000000000000000000000000000000
0000000000000000000000000000000001111111111111111111111111110000000000000000000000000000000000000000
0000000000000000000000000000000011111111111111111111111111110000000000000000000000000000000000000000
0000000000000000000000000000000000111111111111111111111111000000000000000000000000000000000000000000
```

### Dependencies 
- [LLVM 16.0.0](https://github.com/llvm/llvm-project/tree/release/16.x)
- [Premake5](https://github.com/premake/premake-core)
