#include "compiler.h"
#include "compiler/compiler/compilation_context.h"

#include <tokenizer/tokenizer.h>
#include <parser/parser.h>
#include <type_checker/type_checker.h>
#include <ir_translator/ir_translator.h>

#include <utility/filesystem/file.h>
#include <utility/string_helper.h>
#include <utility/timer.h>

namespace sigma {
	void compiler::compile(const filepath& path, ir::target target) {
		compiler(path, target).compile();
	}

	compiler::compiler(const filepath& path, ir::target target)
		: m_path(path), m_target(target) {}

	void compiler::compile() const {
		utility::console::println("compiling file: {}", m_path.string());

		verify_file(m_path);
		const std::string file = utility::file::read_text_file(m_path);
		auto [tokens, symbols] = tokenizer::tokenize(file);

		compilation_context context{
			.string_table = symbols,
			.tokens = tokens
		};

		context.ast = parser::parse(context);
		context.print_ast();

		type_checker::type_check(context);
		ir::module module = ir_translator::translate(context, m_target);
		module.compile();

		// emit the object file
		const filepath object_path = get_object_file_path();
		emit_object_file(module, object_path);
	}

	void compiler::verify_file(const filepath& path) {
		ASSERT(path.extension() == ".s", "invalid file extension detected");
		ASSERT(std::filesystem::is_regular_file(path), "invalid file detected");
	}

	auto compiler::get_object_file_path(const std::string& name) const -> filepath {
		ASSERT(!name.empty(), "cannot create an object file with no name");

		static const char* object_formats[] = {
			".obj", // WINDOWS
			".o"    // LINUX
		};

		const char* format = object_formats[static_cast<u8>(m_target.get_system())];
		return  m_path.parent_path() / (name + format);
	}

	void compiler::emit_object_file(ir::module& module, const filepath& path) {
		utility::file::write(module.generate_object_file(), path);
	}
} // namespace sigma
