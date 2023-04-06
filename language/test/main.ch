// todo: add support for operator-assignment.

// priority todo list: 
// - add support for multiple scopes in every function

i32 g = 10;

i32 func() {
    i32 a = g;
}

i32 main() {
    i32 a = 3 * func();
}