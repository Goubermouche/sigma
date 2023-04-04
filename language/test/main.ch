// todo: only add global variables to the initialization list when we're setting it to a non-const value.
//     optimisation: use llvm.global_ctors, this would also solve the thing above.
// todo: add support for operator-assignment.

// priority todo list: 
// - add support for multiple scopes in every function

i32 a = 1;
i32 b = a;

i32 main() {
    i32 c = a * (b / 3);
    i32 kys = 68;
}