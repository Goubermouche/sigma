//#include "compiler/compiler.h"
#include <intermediate_representation/module.h>
#include <utility/filesystem/new/file.h>
#include <utility/timer.h>

using namespace utility::types;

// todo: 
// - rework the console so that it supports multiple stream types
// - make the pe executable format derive from the file class
// - replace print methods with overloaded friend methods

/**
 * \brief Runs the compiler. 
 * \param argc Argument count
 * \param argv Argument values
 * \return Status code.
 */
auto main(i32 argc, char* argv[]) -> i32 {
	// TODO:
	// - MOVE LOGIC OVER TO THE FUNCTION STRUCT
	// - remove the forward lists

	SUPPRESS_C4100(argc);
	SUPPRESS_C4100(argv);

	utility::console::set_output_stream(std::cout);

	utility::timer timer;
	timer.start();

	ir::module m;

	// add function
	const auto add_f = m.create_function("add", { I32_TYPE , I32_TYPE }, { I32_TYPE });

	const auto a = add_f->get_parameter(0);
	const auto b = add_f->get_parameter(1);
	const auto add = add_f->create_add(a, b);
	add_f->create_ret({ add });

	const auto main_f = m.create_function("main", {}, { I32_TYPE });

	const auto a_val = main_f->create_signed_integer(100, 32);
	const auto b_val = main_f->create_signed_integer(200, 32);

	const auto returns = main_f->create_call(add_f, { a_val , b_val });
	main_f->create_ret(returns);

	auto compilation_result = m.compile(ir::arch::x64, ir::system::windows);

	utility::console::out 
		<< "compilation finished (" 
		<< utility::console::precision(3)
		<< timer.elapsed<std::chrono::duration<f64>>()
		<< "s)\n";

	std::cout << compilation_result.assembly << '\n';

	std::cout << "bytecode:\n";
	for(utility::byte byte : compilation_result.bytecode) {
		std::cout << byte.to_hex() << ' ';
	}
	std::cout << '\n';

	// const auto serialization_result_asm = utility::file::write(
	// 	compilation_result.get_value()->assembly_output, "./test/assembly.asm"
	// );
	// 
	// ASSERT(!serialization_result_asm.has_error(), "unhandled error encountered");

	return 0;

	//const std::vector<utility::byte> bytes = assembly.get_code();

	//for (const auto& byte : bytes) {
	//	utility::console::out << "0x" << byte.to_hex() << '\n';
	//}

	//code_generator::executable exe("./test/app.exe", bytes);

	//sigma::program_options options;
	//sigma::command& compile_command = options.add_command(
	//	"compile", {
	//		.description = "compile the given source file using the specified arguments",
	//		.action = [](sigma::argument_list& arguments)->i32 {
	//			// parse arguments to settings
	//			const auto settings_parse_result = sigma::compiler_settings::parse_argument_list(
	//				arguments
	//			);

	//			if(settings_parse_result.has_error()) {
	//				settings_parse_result.get_error()->print();
	//			}

	//			// construct the compiler object
	//			sigma::compiler compiler;
	//			const auto compilation_result = compiler.compile(
	//				settings_parse_result.get_value()
	//			);

	//			// check for compilation errors
	//			if (compilation_result.has_error()) {
	//				compilation_result.get_error()->print();
	//				return 1;
	//			}

	//			return 0;
	//		}
	//	}
	//);

	//compile_command.add_argument(
	//	"source", {
	//	.short_tag = "s",
	//	.description = "specify the input file the compiler should compile",
	//	.expected = "path",
	//	.required = true
	//});

	//compile_command.add_argument(
	//	"output", {
	//	.short_tag = "o",
	//	.description = "specify the output files (.exe, .o, .llvm)",
	//	.expected = "path",
	//	.required = true,
	//	.value_range = { 1, std::numeric_limits<u64>::max() }
	//});

	//// performance optimization
	//compile_command.add_argument("optimize0", {
	//	.short_tag = "o0",
	//	.description = "apply no optimization, reduce compile time",
	//	.required = false,
	//	.default_value = false,
	//	.implicit_value = true,
	//});

	//compile_command.add_argument("optimize1", {
	//	.short_tag = "o1",
	//	.description = "apply basic optimization, reduce code size while limiting the impact on compile time",
	//	.required = false,
	//	.default_value = false,
	//	.implicit_value = true,
	//});

	//compile_command.add_argument("optimize2", {
	//	.short_tag = "o2",
	//	.description = "apply medium optimization, greater impact on compile time",
	//	.required = false,
	//	.default_value = false,
	//	.implicit_value = true,
	//});

	//compile_command.add_argument("optimize3", {
	//	.short_tag = "o3",
	//	.description = "apply aggressive optimization, reduce code size, greatest impact on compile time",
	//	.required = false,
	//	.default_value = false,
	//	.implicit_value = true,
	//});

	//// size optimization
	//compile_command.add_argument("size-optimize0", {
	//	.short_tag = "s0",
	//	.description = "apply no size optimization, reduce compile time",
	//	.required = false,
	//	.default_value = false,
	//	.implicit_value = true,
	//});

	//compile_command.add_argument("size-optimize1", {
	//	.short_tag = "s1",
	//	.description = "apply medium size optimization, greater impact on compile time",
	//	.required = false,
	//	.default_value = false,
	//	.implicit_value = true,
	//});

	//compile_command.add_argument("size-optimize2", {
	//	.short_tag = "s2",
	//	.description = "apply aggressive size optimization, potentially slower code, greatest impact on compile time",
	//	.required = false,
	//	.default_value = false,
	//	.implicit_value = true,
	//});

	//// vectorization
	//compile_command.add_argument("fslp-vectorize", {
	//	.short_tag = "v",
	//	.description = "enable SLP vectorizer, may increase performance in high-data applications",
	//	.required = false,
	//	.default_value = false,
	//	.implicit_value = true,
	//});

	//return options.parse(argc, argv);
}
