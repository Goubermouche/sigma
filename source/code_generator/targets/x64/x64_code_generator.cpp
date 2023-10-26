#include "x64_code_generator.h"

namespace code {
	x64_code_generator::x64_code_generator(
		s_ptr<register_allocator_base> register_allocator
	) : code_generator_base(register_allocator) {}

	void x64_code_generator::compile_function(handle<ir::function> function) {
		utility::console::out
			<< "compiling function: "
			<< function->get_symbol().get_name()
			<< '\n';
	}
}