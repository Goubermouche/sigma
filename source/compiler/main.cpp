//#include "compiler/compiler.h"
#include "code_generator/code_generator.h"
#include "intermediate_code/intermediate_code.h"

using namespace utility::types;

/**
 * \brief Runs the compiler. 
 * \param argc Argument count
 * \param argv Argument values
 * \return Status code.
 */
i32 main(i32 argc, char* argv[]) {
	utility::console::init();
	ir::builder builder;

	const auto function = builder.create_function(
		ir::integer_type::create(32, true), {
			ir::integer_type::create(32, true)
		}, "main"
	);

	const auto block = builder.create_block(function, "entry");
	builder.set_insert_point(block);

	const auto allocation = builder.create_stack_allocation(
		ir::integer_type::create(32, true)
	);

	const auto store = builder.create_store(
		allocation,
		ir::integer_constant::create(
			ir::integer_type::create(32, true),
			999
		)
	);

	const auto load = builder.create_load(
		ir::integer_type::create(32, true), 
		allocation
	);

	const auto add = builder.create_add(
		load,
		function->get_arguments().front()
	);

	const auto ret = builder.create_ret(add);
	builder.print();

	sigma::code_generator generator(builder);
	generator.allocate_registers();

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
