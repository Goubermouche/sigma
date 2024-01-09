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

## Nvidia compilation model
-   Generally speaking, we want to split the host and device codegen (host codegen can be handled by TB).
-   Device codegen
    -   Device IR -> PTX assembly
-   Host codegen
    -   Host IR -> assembly
-   After codegen finishes, we want to determine, whether any kernels are invoked
    -   If there is at least one active kernel, we need to add CUDA context creation and deletion calls (this can be done by linking to `cuda.lib` for now).
-   PE emission
    -   Focus on Windows for now.
    -   Add a .device data segment to the PE file.
    -   Reference specific points of the .device segment when creation kernels using `cuModuleLoadData`.
    -   Link with `cuda.lib` and other necessary libraries.
 
## Links 
-   [**CUDA PTX**](https://docs.nvidia.com/cuda/parallel-thread-execution/index.html)
-   [**OpenCL**](https://www.khronos.org/opencl/)
