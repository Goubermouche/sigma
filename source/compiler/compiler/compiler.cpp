#include "compiler.h"
#include "compiler/compiler/compilation_context.h"

#include <tokenizer/tokenizer.h>
#include <parser/parser.h>
#include <type_checker/type_checker.h>
#include <ir_translator/ir_translator.h>

#include <utility/filesystem/file.h>
#include <utility/string_helper.h>
#include <utility/timer.h>

#define LANG_FILE_EXTENSION ".s"

namespace sigma {
	auto compiler::compile(const compiler_description& description) -> utility::result<void> {
		return compiler(description).compile();
	}

	compiler::compiler(const compiler_description& description)
		: m_description(description) {}

	auto compiler::compile() const -> utility::result<void> {
		utility::console::println("compiling file: {}", m_description.path.string());
		TRY(verify_file(m_description.path));

		// declare a global compilation context
		// NOTE: it may be a good idea to separate our contexts a bit for when we want to add support
		//       for multiple files
		compilation_context context;

		// generate tokens
		TRY(const std::string& file, utility::file::read_text_file(m_description.path));
		TRY(auto tokenized, tokenizer::tokenize(file));

		context.string_table = tokenized.second;
		context.tokens = tokenized.first;

		// parse the token list and generate an AST
		TRY(context.ast, parser::parse(context));

		// context.print_ast();

		// run analysis on the generated AST
		TRY(type_checker::type_check(context));
		TRY(ir::module module, ir_translator::translate(context, m_description.target));

		// compile the generated IR module
		module.compile();

		// emit as an object file
		const filepath object_path = get_object_file_path();
		emit_object_file(module, object_path);
		return SUCCESS;
	}

	auto compiler::verify_file(const filepath& path) -> utility::result<void> {
		if(!utility::fs::exists(path)) {
			return utility::error::create(utility::error::code::FILE_DOES_NOT_EXIST, path.string());
		}

		if(!utility::fs::is_file(path)) {
			return utility::error::create(utility::error::code::EXPECTED_FILE, path.string());
		}

		if(path.extension() != LANG_FILE_EXTENSION) {
			return utility::error::create(utility::error::code::INVALID_FILE_EXTENSION, path.string(), LANG_FILE_EXTENSION);
		}

		return SUCCESS;
	}

	auto compiler::get_object_file_path(const std::string& name) const -> filepath {
		ASSERT(!name.empty(), "cannot create an object file with no name");

		static const char* object_formats[] = {
			".obj", // WINDOWS
			".o"    // LINUX
		};

		const char* format = object_formats[static_cast<u8>(m_description.target.get_system())];
		return m_description.path.parent_path() / (name + format);
	}

	void compiler::emit_object_file(ir::module& module, const filepath& path) {
		utility::file::write(module.generate_object_file(), path);
	}
} // namespace sigma
