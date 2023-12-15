#include "compiler/compiler.h"

#include <utility/macros.h>
using namespace utility::types;

//ast.traverse([&](const handle<ast::node>& node, u16 depth) {
//	 std::cout << std::string(depth * 2, ' ');
//	switch (node->type.type) {
//		case ast::node_type::FUNCTION:
//			std::cout << "FUNCTION " << utility::detail::escape_string(syntax.symbols.get(node->get<ast::function>().identifier_key)) << '\n';
//			return;
//		case ast::node_type::FUNCTION_CALL:
//			std::cout << "FUNCTION_CALL " << utility::detail::escape_string(syntax.symbols.get(node->get<ast::function_call>().callee_identifier_key)) << '\n';
//			return;
//		case ast::node_type::VARIABLE_DECLARATION:
//			std::cout << utility::detail::escape_string(syntax.symbols.get(node->get<ast::variable>().identifier_key)) << '\n';
//			return;
//		case ast::node_type::STRING_LITERAL:
//		case ast::node_type::NUMERICAL_LITERAL:
//			std::cout << '"' << utility::detail::escape_string(syntax.symbols.get(node->get<ast::literal>().value_key)) << "\"\n";
//		return;
//	}
//	std::cout << node->type.to_string() << '\n';
//});

// ./source/compiler/bin/Release/compiler
// ./output/bin/Release/compiler

auto main(i32 argc, char* argv[]) -> i32 {
	SUPPRESS_C4100(argc);
	SUPPRESS_C4100(argv);

	sigma::compiler::compile(
		"./test/main.s", 
		{ sigma::ir::arch::X64, sigma::ir::system::WINDOWS }
	);

	return 0;

	//sigma::ir::target target(sigma::ir::arch::X64, sigma::ir::system::WINDOWS);
	//sigma::ir::module module(target);
	//sigma::ir::builder builder(module);

	//const sigma::ir::function_signature main_func_ty{
	//	.identifier = "main",
	//	.returns = { I32_TYPE }
	//};

	//// main
	//builder.create_function(main_func_ty, sigma::ir::linkage::PUBLIC);

	//const sigma::ir::function_signature printf_func_ty{
	//	.identifier = "printf",
	//	.parameters = { PTR_TYPE },
	//	.returns = { I32_TYPE },
	//	.has_var_args = true
	//};

	//const auto printf_external = builder.create_external(printf_func_ty, sigma::ir::linkage::SO_LOCAL);

	//const auto message_true = builder.create_string("true\n");
	//const auto message_false = builder.create_string("false\n");

	//const auto true_ctrl = builder.create_region();
	//const auto false_ctrl = builder.create_region();
	//const auto after_ctrl = builder.create_region();

	//builder.create_conditional_branch(builder.create_bool(true), true_ctrl, false_ctrl);

	//builder.set_control(false_ctrl);
	//builder.create_call(printf_external, printf_func_ty, { message_false });
	//builder.create_branch(after_ctrl);

	//builder.set_control(true_ctrl);
	//builder.create_call(printf_external, printf_func_ty, { message_true });
	//builder.create_branch(after_ctrl);

	//

	//// end
	//builder.set_control(after_ctrl);
	//builder.create_return({  builder.create_signed_integer(0, 32) });
	//module.compile();

	//auto object_file = module.generate_object_file();

	//auto write_res = utility::file::write(object_file, "./test/a.obj");
	//if (write_res.has_error()) {
	//	utility::console::out << *write_res.get_error() << '\n';
	//}

	//return 0;

	//utility::console::out 
	//	<< "compilation finished (" 
	//	<< utility::console::precision(3)
	//	<< timer.elapsed<std::chrono::duration<f64>>()
	//	<< "s)\n";

	//std::cout << compilation_result.assembly << '\n';

	//std::cout << "bytecode:\n";
	//for(utility::byte byte : compilation_result.bytecode) {
	//	std::cout << byte.to_hex() << ' ';
	//}
	//std::cout << '\n';

	// const auto serialization_result_asm = utility::file::write(
	// 	compilation_result.get_value()->assembly_output, "./test/assembly.asm"
	// );
	// 
	// ASSERT(!serialization_result_asm.has_error(), "unhandled error encountered");

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
