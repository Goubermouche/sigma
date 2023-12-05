# Compiler design
## Background
Let 'device' refer to any processing unit which is designated for usage in kernels (ie. a GPU), and let 'host' refer to a CPU which serves as the main processing unit of the active system.

## Random notes and possible issues
-   Device implementation & kernels
    -   Handling compilation - how do we handle different devices?
        -   Handling different architecture combinations - statically compiled vs JIT (prefer statically compiled).
        -   Something along the lines of OpenCL, but they JIT their kernels on app startup.
    -   How unified should the systems be?
        -   We want clear separation between device and host code.
 
## Links 
-   [**CUDA PTX**](https://docs.nvidia.com/cuda/parallel-thread-execution/index.html)
-   [**OpenCL**](https://www.khronos.org/opencl/)
