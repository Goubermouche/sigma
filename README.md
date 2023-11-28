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
After building the Sigma solution, you can find the generated compiler executable in the `/bin/Release/` folder. To run the executable, use a terminal of your choice. Provide it with a path to a `.s` file and specify the directory where the compiled executable should be output. An example compilation call may look something like this: 
```cmd
compiler.exe path\to\your\source\file.s path\to\your\executable\directory
```
## Todo: 
- [X] Replace LLVM by a basic TB-like code generator
- [ ] Rework the AST structure (Carbon-style flat AST)
- [ ] Clean up the project structure
- [ ] Implement a separate AST checker
- [ ] Full C function toolset
- [X] Support for external files and linking
- [ ] Integral formats (hexadecimal, decimal, octal)
- [ ] Enum support
- [ ] Struct support
- [ ] GPU support with kernels 
## Tooling
- [Syntax highlighting](https://github.com/Goubermouche/sigma-syntax-highlighter)

### Dependencies 
- [LLVM 16.0.0](https://github.com/llvm/llvm-project/tree/release/16.x) (temporary)
- [Premake5](https://github.com/premake/premake-core)
