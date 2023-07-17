<p align="center">
 <a href="https://github.com/Goubermouche/sigma">
    <img src="https://github.com/Goubermouche/sigma/blob/3993a0c5c84eeb64d01f3185c3a0b7aa7d470dec/images/banner2.png" alt="sigma logo">
   </a>
</p>
Sigma is a compiled, statically typed, C-like, data oriented language with a focus on performance and low-level optimization using GPU acceleration via kernels.  

## Getting up and running      

### Windows 10       

<ins>**1. Downloading the repository:**</ins>   
Clone the repository with `git clone https://github.com/Goubermouche/sigma.git`.

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
After building LLVM you'll have to link the necessary libraries and create the basic project structure. To do this, navigate to the root sigma directory and run the [setup.bat](https://github.com/Goubermouche/sigma/blob/1546a311f1e7280321ca556ac2f5c380e62bd97e/setup.bat) script. This will generate the necessary project files for VS 2022, note that if you want to build for something else you'll need to modify the [setup.bat](https://github.com/Goubermouche/sigma/blob/1546a311f1e7280321ca556ac2f5c380e62bd97e/setup.bat) script. 

<ins>**4. Compiling a Sigma file**</ins>   
After building the Sigma solution, you can find the generated compiler executable in the `/bin/Release/` folder. To run the executable, use a terminal of your choice. Provide it with a path to a `.ch` file and specify the directory where the compiled executable should be output. An example compilation call may look something like this: 
```cmd
compiler.exe path\to\your\source\file.ch path\to\your\executable\directory
```
## Language example
Below is a simple example for rendering a slice of a [Mandelbrot set](https://en.wikipedia.org/wiki/Mandelbrot_set) using the Sigma language.
```cpp
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
            printc(get_char(iterations));
        }
    
        printc('\n');
    }
    
    return 0;
}
```
The expected output, after running the generated .exe, looks like this:  
```
,,,,,,,,,--------~~~~~~~~~~~~~~~~~~~~~~~~~~~::::::::::;;;;==!#~$#@!=;:::::::~~~~~~~~----------------
,,,,,,,,-------~~~~~~~~~~~~~~~~~~~~~~~~~~::::::::::::;;;;=!!*#@*.#*!=;;::::::::~~~~~~~--------------
,,,,,,,-----~~~~~~~~~~~~~~~~~~~~~~~~~~~::::::::::::;;;;==*$=.-; ..$$*=;;;;::::::~~~~~~~~~-----------
,,,,,,----~~~~~~~~~~~~~~~~~~~~~~~~~~::::::::::::;;;====!*#.@      #-#!==;;;;;:::::~~~~~~~~~---------
,,,,,----~~~~~~~~~~~~~~~~~~~~~~~~:::::::::::;;;=====!!!*#$,*      @,#*!!=====;;;:::~~~~~~~~~--------
,,,,---~~~~~~~~~~~~~~~~~~~~~~~~::::::::;;;;;=!~-= @##$:;,$:;@    @;=-,@;=*!!!!#~=;::~~~~~~~~~~------
,,,,--~~~~~~~~~~~~~~~~~~~~~~::::::;;;;;;;;===!$,=  ;~,-                 :,@--~-=~!;::~~~~~~~~~~-----
,,,--~~~~~~~~~~~~~~~~~~~~::::;;;;;;;;;;;====**#@:,                        .   #@*=;;::~~~~~~~~~~----
,,--~~~~~~~~~~~~~~~~::::;;=;;;;;;;;;=====!!@#@@!                             :@#!==;;::~~~~~~~~~~---
,,-~~~~~~~~~~~::::::;;==#.*!!!!!!!!!!!!!!**$,**                               $.##$=;:::~~~~~~~~~~--
,-~~~~~~:::::::::;;;;==!#=@@,$#$$-$##***##@!                                     *.=;::::~~~~~~~~~~-
,~~~~::::::::::;;;;;;=!!*#@~@,!#$ :*=~,@@@-,                                    ,#!=;::::~~~~~~~~~~~
,~~::::::::::;;;;;;;=!!*.@-!~          ;~~#                                     !:!;;::::~~~~~~~~~~~
,::::::::::;;=====!*,$#@@:.              @=                                    ~*!=;;:::::~~~~~~~~~~
,:;;;;;=!!====!!!!##.$-!.!                                                    ~#!=;;;:::::~~~~~~~~~~
~*:=#=:: $:-:@*@!*==                                                        :@#!!=;;;:::::~~~~~~~~~~
,:;;;;;=!!====!!!!##.$-!.!                                                    ~#!=;;;:::::~~~~~~~~~~
,::::::::::;;=====!*,$#@@:.              @=                                    ~*!=;;:::::~~~~~~~~~~
,~~::::::::::;;;;;;;=!!*.@-!~          ;~~#                                     !:!;;::::~~~~~~~~~~~
,~~~~::::::::::;;;;;;=!!*#@~@,!#$ :*=~,@@@-,                                    ,#!=;::::~~~~~~~~~~~
,-~~~~~~:::::::::;;;;==!#=@@,$#$$-$##***##@!                                     *.=;::::~~~~~~~~~~-
,,-~~~~~~~~~~~::::::;;==#.*!!!!!!!!!!!!!!**$,**                               $.##$=;:::~~~~~~~~~~--
,,--~~~~~~~~~~~~~~~~::::;;=;;;;;;;;;=====!!@#@@!                             :@#!==;;::~~~~~~~~~~---
,,,--~~~~~~~~~~~~~~~~~~~~::::;;;;;;;;;;;====**#@:,                        .   #@*=;;::~~~~~~~~~~----
,,,,--~~~~~~~~~~~~~~~~~~~~~~::::::;;;;;;;;===!$,=  ;~,-                 :,@--~-=~!;::~~~~~~~~~~-----
,,,,---~~~~~~~~~~~~~~~~~~~~~~~~::::::::;;;;;=!~-= @##$:;,$:;@    @;=-,@;=*!!!!#~=;::~~~~~~~~~~------
,,,,,----~~~~~~~~~~~~~~~~~~~~~~~~:::::::::::;;;=====!!!*#$,*      @,#*!!=====;;;:::~~~~~~~~~--------
,,,,,,----~~~~~~~~~~~~~~~~~~~~~~~~~~::::::::::::;;;====!*#.@      #-#!==;;;;;:::::~~~~~~~~~---------
,,,,,,,-----~~~~~~~~~~~~~~~~~~~~~~~~~~~::::::::::::;;;;==*$=.-; ..$$*=;;;;::::::~~~~~~~~~-----------
,,,,,,,,-------~~~~~~~~~~~~~~~~~~~~~~~~~~::::::::::::;;;;=!!*#@*.#*!=;;::::::::~~~~~~~--------------
,,,,,,,,,--------~~~~~~~~~~~~~~~~~~~~~~~~~~~::::::::::;;;;==!#~$#@!=;:::::::~~~~~~~~----------------
```
## Todo: 
- [ ] Full C function toolset
- [ ] Support for external files and linking
- [ ] Integral formats (base-16 HEX, base-8 HEX)
- [ ] Enum support
- [ ] Struct support
- [ ] GPU support with kernels 
## Tooling
- [Syntax highlighting](https://github.com/Goubermouche/sigma-syntax-highlighter)

### Dependencies 
- [LLVM 16.0.0](https://github.com/llvm/llvm-project/tree/release/16.x)
- [Premake5](https://github.com/premake/premake-core)
