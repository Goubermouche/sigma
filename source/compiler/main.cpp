//#include "compiler/compiler.h"
#include <intermediate_representation/module.h>
#include <utility/filesystem/new/file.h>

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
i32 main(i32 argc, char* argv[]) {
	SUPPRESS_C4100(argc);
	SUPPRESS_C4100(argv);

	utility::console::set_output_stream(std::cout);
	ir::module m;

	m.create_function("test", { I32_TYPE });

	const auto temporary_a = m.create_local(4, 4);
	const auto temporary_b = m.create_local(4, 4);
	const auto integer_a = m.create_signed_integer(100, 32);
	const auto integer_b = m.create_signed_integer(200, 32);

	m.create_store(temporary_a, integer_a, 4, false);
	m.create_store(temporary_b, integer_b, 4, false);
	m.create_ret({ m.create_add(temporary_a, temporary_b) });
	const auto print_result = m.print_node_graph("./test/app.dot");
	ASSERT(!print_result.has_error(), "unhandled error encountered");

	const auto compilation_result = m.compile(
		ir::arch::x64, ir::system::windows
	);

	ASSERT(!compilation_result.has_error(), "unhandled error encountered");

	const auto serialization_result_asm = utility::file::write(
		compilation_result.get_value()->assembly_output, "./test/assembly.txt"
	);

	ASSERT(!serialization_result_asm.has_error(), "unhandled error encountered");

	const auto serialization_result_code = utility::file::write(
		compilation_result.get_value()->bytecode, "./test/bytecode.txt"
	);

	ASSERT(!serialization_result_code.has_error(), "unhandled error encountered");


	// utility::console::out << "\nbyte code:\n\n";
	// for(const utility::byte b : compilation_result.get_value()) {
	// 	utility::console::out << b.to_hex() << ' ';
	// }

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
