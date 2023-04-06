// todo: only add global variables to the initialization list when we're setting it to a non-const value.
//     optimisation: use llvm.global_ctors, this would also solve the thing above.
// todo: add support for operator-assignment.

// priority todo list: 
// - add support for multiple scopes in every function

i32 func() {
    i32 a = 2;
}

i32 main() {
    
    i32 a = 4;
}
