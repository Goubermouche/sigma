#include "compiler.h"

#include <ir_translator/ir_translator.h>
#include <type_checker/type_checker.h>
#include <tokenizer/tokenizer.h>
#include <parser/parser.h>

#include "compiler/compiler/compilation_context.h"
#include "compiler/compiler/diagnostics.h"

#define LANG_FILE_EXTENSION ".s"

namespace sigma {
	auto compiler::compile(const compiler_description& description) -> utility::result<void> {
		return compiler(description).compile();
	}

	compiler::compiler(const compiler_description& description)
		: m_description(description) {}

	auto compiler::compile() -> utility::result<void> {
		utility::console::print("compiling file: {} ({})\n", m_description.source_path, m_description.emit_path);

		TRY(verify_file(m_description.source_path));
		TRY(m_emit_target, get_emit_target_from_path(m_description.emit_path));

		// frontend
		// TODO: the entire frontend can be multi-threaded
		frontend_context frontend;

		// generate the AST
		TRY(const std::string file, utility::fs::load(m_description.source_path));
		TRY(tokenizer::tokenize(file, &m_description.source_path, frontend));
		TRY(parser::parse(frontend));

		// backend
		// at this point we want to merge all frontend contexts into the backend context
		backend_context backend(frontend.syntax, m_description.target);

		// run analysis on the generated AST
		TRY(type_checker::type_check(backend));
		TRY(ir_translator::translate(backend));

		// compile the generated IR module
		backend.module.compile();

		//emit as an object file
		if(m_emit_target == emit_target::OBJECT) {
			TRY(emit_object_file(backend.module, m_description.emit_path));
		}

		return SUCCESS;
	}

	auto compiler::verify_file(const filepath& path) -> utility::result<void> {
		if(!path.exists()) {
			return error::emit(error::code::FILE_DOES_NOT_EXIST, path);
		}

		if(!path.is_file()) {
			return error::emit(error::code::EXPECTED_FILE, path);
		}

		if(path.get_extension() != LANG_FILE_EXTENSION) {
			return error::emit(error::code::INVALID_FILE_EXTENSION, path, LANG_FILE_EXTENSION);
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
		return m_description.source_path.get_parent_path() / (name + format);
	}

	auto compiler::emit_object_file(ir::module& module, const filepath& path) -> utility::result<void> {
		return utility::fs::write(path, module.generate_object_file());
	}

	auto compiler::get_emit_target_from_path(const filepath& path) const -> utility::result<emit_target> {
		if(path.get_extension() == ".exe") {
			return emit_target::EXECUTABLE;
		}

		if(path.get_extension() == ".obj") {
			ASSERT(m_description.target.get_system() == ir::system::WINDOWS, "incompatible target and object format");
			return emit_target::OBJECT;
		}

		if (path.get_extension() == ".o") {
			ASSERT(m_description.target.get_system() == ir::system::LINUX, "incompatible target and object format");
			return emit_target::OBJECT;
		}

		NOT_IMPLEMENTED();
		return emit_target::NONE;
	}
} // namespace sigma
