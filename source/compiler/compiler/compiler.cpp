#include "compiler.h"
#include "compiler/compiler/compilation_context.h"

#include <tokenizer/tokenizer.h>
#include <parser/parser.h>
#include <type_checker/type_checker.h>
#include <ir_translator/ir_translator.h>

#include <utility/filesystem/file.h>
#include <utility/string_helper.h>

#define LANG_FILE_EXTENSION ".s"

namespace sigma {
	auto compiler::compile(const compiler_description& description) -> utility::result<void> {
		return compiler(description).compile();
	}

	compiler::compiler(const compiler_description& description)
		: m_description(description) {}

	auto compiler::compile() const -> utility::result<void> {
		utility::console::print("compiling file: {}\n", m_description.path.string());

		ASSERT(m_description.emit != emit_target::EXECUTABLE, "executable support not implemented");
		TRY(verify_file(m_description.path));

		// frontend
		// TODO: the entire frontend can be multi-threaded
		frontend_context frontend;

		// generate tokens
		TRY(const std::string file, utility::file::read_text_file(m_description.path));

		TRY(auto tokenized, tokenizer::tokenize(file));

		frontend.syntax.string_table = tokenized.second;
		frontend.tokens = tokenized.first;

		TRY(frontend.syntax.ast, parser::parse(frontend));

		// backend
		backend_context backend(std::move(frontend.syntax), m_description.target);

		// run analysis on the generated AST
		TRY(type_checker::type_check(backend));
		TRY(ir_translator::translate(backend));

		// compile the generated IR module
		backend.module.compile();

		// emit as an object file
		if(m_description.emit == OBJECT) {
			const filepath object_path = get_object_file_path();
			emit_object_file(backend.module, object_path);
		}
		
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
