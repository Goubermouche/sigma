#include "compiler/compiler.h"
#include <utility/shell.h>

using namespace utility::types;

i32 compile(const parametric::parameters& params) {
	const sigma::compiler_description description {
		.source_path = params.get<filepath>("file"),
		.emit_path = params.get<filepath>("emit"),

		// default to x64 win for now
		.target = { sigma::ir::arch::X64, sigma::ir::system::WINDOWS },
	};

	// compile the specified description, check for errors after we finish
	const auto result = sigma::compiler::compile(description);

	if (result.has_error()) {
		utility::console::printerr("{}\n", result.get_error().get_message());
		return 1;
	}

	return 0;
}

i32 show_docs(const parametric::parameters& params) {
	SUPPRESS_C4100(params);

	// TODO: add a link to the relevant github wiki
	const std::string link = "https://github.com/Goubermouche/sigma";

	if(utility::shell::open_link(link) == 0) {
		return 0;
	}

	std::cout << std::format("error: unable to open the documentation link ({})\n", link);
	return 1;
}

auto main(i32 argc, char* argv[]) -> i32 {
	parametric::program program;

	// compilation
	auto& compile_command = program.add_command("compile", "compile the specified source file", compile);

	compile_command.add_positional_argument<filepath>("file", "source file to compile");
	compile_command.add_flag<filepath>("emit", "filepath to emit to", "e", "./a.obj");
	compile_command.add_flag<sigma::ir::arch>("arch", "CPU architecture to compile for [x64]");
	compile_command.add_flag<sigma::ir::system>("system", "operating system to compile for [windows, linux]");

	// TODO: add support for emitting multiple files at once

	// documentation
	program.add_command("docs", "show project documentation", show_docs);

	return program.parse(argc, argv);
}
