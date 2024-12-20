<p align="center">
 <a href="https://github.com/Goubermouche/sigma">
    <img src="/images/banner_3.png" alt="sigma logo" width="650cm">
   </a>
</p>

<div align="center">
 
[![Linux](https://github.com/Goubermouche/sigma/actions/workflows/linux.yml/badge.svg)](https://github.com/Goubermouche/sigma/actions/workflows/linux.yml)   [![Windows](https://github.com/Goubermouche/sigma/actions/workflows/windows.yml/badge.svg)](https://github.com/Goubermouche/sigma/actions/workflows/windows.yml)
</div>

> [!IMPORTANT]
> I've taken a break from developing Sigma, but the project isn't dead and will be worker on eventually, in some shape or form.


<p align="center">
Sigma is a compiled, statically typed, C-like, data oriented language with a focus on performance and low-level optimization using GPU acceleration via kernels. Sigma is currently under heavy development and isn't ready for stable use. 
</p>

## Getting up and running      
Get started by running:
```shell
# Download Sigma's source code
$ git clone https://github.com/Goubermouche/sigma.git --recursive
$ cd sigma

# Generate project files
$ premake5 vs2022
```
After downloading and building the project you can compile a .s file [WIP]: 
```shell
# Compile main.s as a Sigma source file
$ sigma compile main.s
```

## Project status
As Sigma is currently under heavy development, it isn't ready for stable use, and many breaking changes may (and most likely will) be made. Please note that while the end goal is to target GPU's and other massivelly parallel targets, the current goal is to get a basic CPU-oriented language going, and there isn't a hard timeframe set - for more details see the current status bellow: 
-   TODO:
    -   Project cleanup
    -   Better object file emitter (Windows)
    -   First PTX tests 
-   Future:
    -   Unified package manager
    -   Official GPU support via kernels

## Contributing
Contributions, either to the compiler implementation, or related to language design are greatly appreciated and welcome.
-   [**Contribute to the compiler implementation**](/documents/CONTRIBUTING.md#ontributing_to_the_compiler_implementation): Bug reports, optimizations, new features.
-   [**Contribute to the language design**](/documents/CONTRIBUTING.md#contributing_to_the_language_design): Feedback on the language design, new feature request, interesting ideas.

## Tooling
Sigma strives to be a fully functional enviroment, for this purpose we're developing several tools which aim to ease entry cost and improve the developer experience. As more tools become available they will be added to the list bellow: 
-   [**Syntax highlighting**](https://github.com/Goubermouche/sigma-syntax-highlighter) (Also available on the VS Code marketplace)

### Dependencies 
-   [**Premake5**](https://github.com/premake/premake-core)
-   [**TB**](https://github.com/RealNeGate/Cuik/tree/master/tb) (indirect, Sigma contains a 'C++ reimplementation' of some TB systems)

