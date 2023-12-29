#include "compiler/compiler.h"
#include <intermediate_representation/builder.h>

#include "utility/filesystem/file.h"

using namespace utility::types;

auto main(i32 argc, char* argv[]) -> i32 {
	SUPPRESS_C4100(argc);

	const sigma::compiler_description description {
	.path = argv[1],
	.target = { sigma::ir::arch::X64, sigma::ir::system::WINDOWS }
	};

	// compile the specified description, check for errors after we finish
	PRINT_ERROR(sigma::compiler::compile(description));
	return 0;

	//sigma::ir::target target(sigma::ir::arch::X64, sigma::ir::system::WINDOWS);
	//sigma::ir::module module(target);
	//sigma::ir::builder builder(module);

	//// printf
	//const sigma::ir::function_signature printf_func_ty{
	//	.identifier = "printf",
	//	.parameters = { PTR_TYPE },
	//	.returns = { I32_TYPE },
	//	.has_var_args = true
	//};

	//const auto printf_external = builder.create_external(printf_func_ty, sigma::ir::linkage::SO_LOCAL);

	//// test
	//const sigma::ir::function_signature test_func_ty {
	//	.identifier = "test",
	//	.parameters = { },
	//	.returns = { I32_TYPE },
	//	.has_var_args = false
	//};

	//const auto test = builder.create_function(test_func_ty, sigma::ir::linkage::PUBLIC);
	//const auto message = builder.create_string("test\n");
	//builder.create_call(printf_external, printf_func_ty, { message });
	//builder.create_return({ builder.create_signed_integer(0, 32) });

	//// main
	//const sigma::ir::function_signature main_func_ty{
	//.identifier = "main",
	//.returns = { I32_TYPE }
	//};

	//builder.create_function(main_func_ty, sigma::ir::linkage::PUBLIC);
	//builder.create_call(test, {});
	//builder.create_return({ builder.create_signed_integer(0, 32) });

	//module.compile();

	//auto object_file = module.generate_object_file();
	//utility::file::write(object_file, "./test/a.obj");
	return 0;
}
