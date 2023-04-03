// todo: only add global variables to the initialization list when we're setting it to a non-const value
// todo: add support for operator-assignment
// todo: add support for underscores in variable names 

i32 a = 1;
i32 b = 2 + a;

i32 main() {
	i32 c; 
	c = a * b + 1;
}