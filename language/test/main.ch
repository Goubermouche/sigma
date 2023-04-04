// todo: only add global variables to the initialization list when we're setting it to a non-const value.
//     optimisation: use llvm.global_ctors, this would also solve the thing above.
// todo: add support for operator-assignment.

// priority todo list: 
// - separate the variable class into local and global variable nodes

i32 a = 1;
i32 b = a * a;

i32 main() {
    i32 value = a - (b % 3);
}