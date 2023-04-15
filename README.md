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

### Dependencies 
- [LLVM 16.0.0](https://github.com/llvm/llvm-project/tree/release/16.x)
- [Premake5](https://github.com/premake/premake-core)
