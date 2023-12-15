#include "compiler.h"
#include "compiler/compiler/compilation_context.h"

#include "tokenizer/tokenizer.h"
#include "parser/parser.h"
#include "type_checker/type_checker.h"
#include "ir_translator/ir_translator.h"

#include "utility/filesystem/new/file.h"
#include "utility/string_helper.h"
#include "utility/timer.h"

namespace sigma {
	void compiler::compile() {
		const auto file = utility::file::read_text_file("/home/goubermouche/dev/projects/sigma/source/compiler/test/main.s");

		utility::timer total_timer;
		utility::timer tokenizer_timer;
		utility::timer parser_timer;
		utility::timer type_checker_timer;
		utility::timer ir_translator_timer;
		utility::timer codegen_timer;

		total_timer.start();

		// tokenizer
		tokenizer_timer.start();
		auto [tokens, symbols] = tokenizer::tokenize(file);

		compilation_context context {
			.symbols = symbols,
			.tokens = tokens
		};

		utility::console::println("{:<30}{:.3f}s", "tokenizer finished", tokenizer_timer.elapsed_seconds());

		// parser
		parser_timer.start();
		context.ast = parser::parse(context);
		utility::console::println("{:<30}{:.3f}s", "parser finished", parser_timer.elapsed_seconds());

		// type checker
		type_checker_timer.start();
		type_checker::type_check(context);
		utility::console::println("{:<30}{:.3f}s", "type checker finished", type_checker_timer.elapsed_seconds());

		// ir translator
		ir_translator_timer.start();
		auto module = ir_translator::translate(context, { ir::arch::X64, ir::system::WINDOWS });
		utility::console::println("{:<30}{:.3f}s", "IR translator finished", ir_translator_timer.elapsed_seconds());

		// codegen
		codegen_timer.start();
		module.compile();
		utility::console::println("{:<30}{:.3f}s", "codegen finished", codegen_timer.elapsed_seconds());

		utility::console::println("{}", std::string(36, '-'));
		utility::console::println("{:<30}{:.3f}s", "compilation finished", total_timer.elapsed_seconds());

		auto object_file = module.generate_object_file();
		utility::file::write(object_file, "/home/goubermouche/dev/projects/sigma/source/compiler/test/a.obj");
	}
} // namespace sigma

