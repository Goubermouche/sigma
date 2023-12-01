# Language design [WIP]

## Background
Sigma aims to be a performant, low level language which follows in the steps of C and GLSL, with a bit of C++ sprinkled into it. Whilst C++ is also a source of inspiration I don't currently plan on adding a standard library. Things such as polymorphism and other performance hogs which aren't suitable to the target platform will be omitted as well.

## File structure
Each Sigma (`.s`) will contain a list of includes at the top of the file (this could be made a requirement, but I haven't decided so far). The entire system will aim to remove header files altogether. 

## Including external files
Including external files in a given `.s` file can be achieved using the `#include` keyword, like so:
```cpp
 // includes and essentially copies the utilities.s file to the target file at the position of the include directive
#include<utilities.s>
```

## Builtin types
Sigma will offer a number of builtin types, which can be seen bellow: 
```rs
// signed integer types
i8, i16, i32, i64

// unsigned integer types
u8, u16, u32, u64

// floating-point types
f32, f64

// text
char

// booleans
bool

// untyped data (same as void in C)
void 
```
Each type class has a literal tied to it, as seen bellow: 
```cpp
// signed integers
42

// unsigned integers
100u

// f32's 
3.1415f

// f64's 
2.7182

// hexadecimal integers
0xFF512

// binary integers
0b01010110

// characters
's'

// string literals (char* or char[])
// string literals are zero terminated automatically
'Sigma' 
```

Additionally, any type can be made into a pointer, like so: 
```rs
i8* // pointer to an i8
char** // pointer to a pointer of chars / array of strings
etc.
```
and static arrays, which will be allocated on the stack, or on the heap, if enough stack space isn't available: 
```rs
i8[100] // static array of 100 i8's
f32[10, 10] // 10*10 matrix of f32's
```

There will be special, GPU-only types available when we start supporting GPU's and kernels (textures, global memory, group shared memory, etc.), however this is just speculation and isn't being worked on actively.

## Variable declarations
Currently, there exist two variants of what variable declaration will look like: 
```rs
// variant A
my_int : u8;
my_str : char*;

// variant B
u8 my_int;
char* my_int;
```
Note that, in order to spare space, most examples containing variable declarations will use the second variant, but both are interchangeable. Additionally, it is undecided whether variables will be const-by-default, if this were to be the case though, the `mut` keyword would be used to declare a non-const variable: 
```rs
// variant A
my_int : mut u8;

// variant B
mut u8 my_int;
```
Variables **won't** be zero-initialized, unless the programmer specifies them to be: 
```rs
// variant A
my_int : u8 = { 0 };
my_int : u8 = 0;
my_arr : u64[10] = { 0 }; // static array of 10 zero-initialized u64's

// variant B
u8 my_int = { 0 };
u8 my_int = 0;
u8[10] my_int = { 0 };
```

## Function declarations
Each function can have at most one return type, a number of parameters, and a function body, like so: 
```rs
// variant A
add(a : f32, b : f32) : f32 {
    // function body
}

// variant B
f32 add(f32 a, f32 b) {
    // function body
}
```
I'll most likely add support for 'function tags', which will allow the programmer to specify how a language should be handled by the compiler, these could include [WIP]:
-   **`@inline`**: Specifies, that the function should be inlined in **all cases** (This may result in spills - in this case the register allocator will emit a warning).
-   **`@host`**: Declares the function as a CPU-only function, meaning there will only exist a CPU-bytecode version of it (and thus only CPU-side code can run it)
-   **`@device`**: Declares the function as a GPU-only function, meaning there will only exist a CPU-bytecode version of it (and thus only GPU-side code can run it)
-   **`@global`**: Declares the function as both a CPU and GPU function, meaning the compiler will compile a GPU and CPU version of it, which results in a function that is callable both from the CPU, and the GPU.

## Structures
Similar to C, Sigma will offer a way of bundling data together: 
```rs
struct vec3 {
    f64 x;
    f64 y;
    f64 z;
}
```
Structures may be initialized using designated initializers, in a similar fashion to C: 
```rs
vec3 position = { .x = 0.0, .y = 10.0, z. 20.0 };
vec3 position = { .x = 0.0, .z = 20.0, y. 10.0 }; // order does not matter
```
Or using undesignated initializers: 
```rs
vec3 position = { 0.0, 10.0, 20.0 }; // x = 0.0, y = 10.0, z = 20.0
vec3 position = { 0.0, 20.0, 10.0 }; // x = 0.0, y = 20.0, z = 10.0
```
Or zero-initialized:
```rs
vec3 position = { 0 } // x = 0.0, y = 0.0, z = 0.0
```

> :warning: **The following section is probably going to undergo major changes.**

Additionally, structures will support a basic base-class system. The goal of this system is to reduce the duplication of code. In order to implement a basic visibility system, the following keywords can be used:
-   **`public`**: The members under this keyword are considered 'public', and can be accessed by from everywhere.
-   **`private`**: The members under this keyword are considered 'private', and can only be accessed by the structure they have been declared in.
-   **`protected:`** The members under this keyword are considered 'protected', and can only be accessed by the structure they have been declared in and every child structure.

Note that all members of a structure are `public` by default. An example of the aforementioned child-parent structure can be seen bellow: 
```rs
struct vec3 {
    f64 x;
    f64 y;
    f64 z;
}

struct object {
protected: 
    f64 mass;
    vec3 position;
}

// 'cube' use 'object' as its base class, meaning it also contains the mass and position members
struct cube : a { 
private:
    u8 height;
}

// mass = 11.11, position = { 0.0, 1.0, 2.0 }, height = 10
cube my_cube = { 11.11, { 0.0, 1.0, 2.0 }, 10 } 
```
Structures may also be declared with methods, which are essentially functions that are tied to an instance of the given structure. Methods implicitly have access to all member variables.
```rs
struct vec3 {
    f64 x;
    f64 y;
    f64 z;
}

struct object {
    void translate() {
        position.x += 10.0;
    }
private:
    vec3 position;
}

object obj = { 0 };
obj.translate(); // position = { 0.0, 10.0, 0.0 }
```
