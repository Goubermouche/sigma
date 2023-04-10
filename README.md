## Channel language

## Getting up and running
### Windows 10
<ins>**1. Downloading the repository:**</ins>   
Clone the repository with `git clone https://github.com/Goubermouche/channel-language.git`.

<ins>**2. Building LLVM:**</ins>   
In order to build the project you will need to build [LLVM](https://llvm.org/) from source, to do this follow the instructions provided by LLVM and build the `ALL_TARGETS` project in the Release configuration. Note that when building you will also have to build the `clang` project by providing the following cmake flag: `-DLLVM_ENABLE_PROJECTS="clang"`. 

<ins>**3. Including LLVM:**</ins>   
After building LLVM you'll need to link against several LLVM libraries, start by adding two additional include directories to `compiler > Properties > Configuration Properties > C/C++ > General > Additional Include Directories` - one for the llvm include directory (llvm-project/llvm/include) and one for the clang include directory (llvm-project/clang/include). Note that in the case of clang you may need also need to add an additional include directory for the built files. Finally, disable SDL checks by going to `compiler > Properties > Configuration Properties > C/C++ > General > SDL checks` and selecting the `No (/sdl-)` option. 
