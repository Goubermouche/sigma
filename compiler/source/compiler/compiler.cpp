#include "compiler.h"

#include "lexer/char_by_char_lexer/char_by_char_lexer.h"
#include "parser/recursive_descent_parser/recursive_descent_parser.h"
#include "code_generator/basic_code_generator/basic_code_generator.h"

#include "utility/timer.h"

// llvm
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Host.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

// clang
#include <clang/Driver/Driver.h>
#include <clang/Driver/Compilation.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>

#include "code_generator/abstract_syntax_tree/keywords/file_include_node.h"

namespace sigma {
	compiler::compiler(
		compiler_settings settings
	) : m_settings(settings) {
		// initialize the default compilation step generators 
		set_lexer<char_by_char_lexer>();
		set_parser<recursive_descent_parser>();
		set_code_generator<basic_code_generator>();
	}

	error_result compiler::compile(
		const filepath& root_source_path,
		const filepath& target_executable_directory
	) {
		m_root_source_path = root_source_path;
		m_target_executable_directory = target_executable_directory;

		timer m_compilation_timer;
		m_compilation_timer.start();

		// verify the root source file
		if (auto source_file_error = verify_source_file(
			m_root_source_path
		)) {
			return source_file_error; // return on failure
		}

		console::out
			<< "compiling file '"
			<< m_root_source_path
			<< "'\n";

		// generate the module
		auto module_generation_result = generate_module(
			m_root_source_path
		);

		if (!module_generation_result) {
			return module_generation_result.error(); // return on failure
		}

		// verify the executable directory
		if (auto executable_directory_error = verify_folder(
			m_target_executable_directory
		)) {
			return executable_directory_error; // return on failure
		}

		// compile the module into an executable
		if (auto compilation_error = compile_module(
			module_generation_result.value()
		)) {
			return compilation_error; // return on failure
		}

		console::out
			<< color::green
			<< "compilation finished ("
			<< m_compilation_timer.elapsed()
			<< "ms)\n"
			<< color::white;

		return {};
	}

	std::expected<
		std::shared_ptr<llvm_context>,
		error_msg
	> compiler::generate_module(
		const filepath& source_path
	) const {
		// tokenize the source file
		timer lexer_timer;
		lexer_timer.start();

		const std::shared_ptr<lexer> lexer = m_lexer_generator();
		if(auto set_source_error = lexer->set_source_filepath(
			source_path
		)) {
			return std::unexpected(set_source_error.value());
		}

		if (auto tokenization_error = lexer->tokenize()) {
			return std::unexpected(
				tokenization_error.value()
			); // return on failure 
		}

		console::out
			<< "lexing finished ("
			<< lexer_timer.elapsed()
			<< "ms)\n";

		// generate the AST
		timer parser_timer;
		parser_timer.start();

		const std::shared_ptr<parser> parser = m_parser_generator();
		parser->set_token_list(
			lexer->get_token_list()
		);

		if (auto parser_error = parser->parse()) {
			return std::unexpected(
				parser_error.value()
			); // return on failure 
		}

		console::out
			<< "parsing finished ("
			<< parser_timer.elapsed()
			<< "ms)\n";

		// 
		for(node* n : *parser->get_abstract_syntax_tree()) {
			if(const auto* include = dynamic_cast<file_include_node*>(n)) {
				console::out
					<< color::red
					<< include->get_path()
					<< color::white
					<< '\n';
			}
		}

		parser->get_abstract_syntax_tree()->print_nodes();

		// generate the module
		timer code_generator_timer;
		code_generator_timer.start();
		const std::shared_ptr<code_generator> code_generator = m_code_generator_generator();
		code_generator->set_abstract_syntax_tree(
			parser->get_abstract_syntax_tree()
		);

		if (auto visitor_error_message = code_generator->generate()) {
			return std::unexpected(
				visitor_error_message.value()
			); // return on failure 
		}

		console::out
			<< "codegen finished ("
			<< code_generator_timer.elapsed()
			<< "ms)\n";

		// code_generator->get_llvm_context()->print_intermediate_representation();
		return code_generator->get_llvm_context();
	}

	error_result compiler::compile_module(
		const std::shared_ptr<llvm_context>& llvm_context
	) const {
		const std::string target_triple = llvm::sys::getDefaultTargetTriple();

		// initialize LLVM targets
		llvm::InitializeAllTargetInfos();
		llvm::InitializeAllTargets();
		llvm::InitializeAllTargetMCs();
		llvm::InitializeAllAsmParsers();
		llvm::InitializeAllAsmPrinters();

		// create the target machine 
		std::string error;
		const auto target = llvm::TargetRegistry::lookupTarget(
			target_triple, 
			error
		);

		constexpr auto cpu = "generic";
		constexpr auto features = "";

		const llvm::TargetOptions target_options;
		constexpr auto relocation_model = llvm::Optional<llvm::Reloc::Model>();
		const auto target_machine = target->createTargetMachine(
			target_triple,
			cpu,
			features,
			target_options, 
			relocation_model
		);


		llvm_context->get_module()->setDataLayout(
			target_machine->createDataLayout()
		);

		llvm_context->get_module()->setTargetTriple(
			target_triple
		);

		const std::string o_file = m_target_executable_directory.string() + "a.o";
		const std::string exe_file = m_target_executable_directory.string() + "a.exe";

		// generate the .o file
		{
			std::error_code error_code;
			llvm::raw_fd_ostream dest(
				o_file, 
				error_code, 
				llvm::sys::fs::OF_None
			);

			llvm::legacy::PassManager pass_manager;
			llvm::PassManagerBuilder builder;

			// add optimization passes
			builder.OptLevel = static_cast<u32>(m_settings.optimization_level);
			builder.SizeLevel = static_cast<u32>(m_settings.size_optimization_level);
			builder.Inliner = llvm::createFunctionInliningPass(
				builder.OptLevel, 
				builder.SizeLevel,
				false
			);

			builder.LoopVectorize = m_settings.vectorize;
			builder.SLPVectorize = m_settings.vectorize;
			builder.populateModulePassManager(
				pass_manager
			);

			if (target_machine->addPassesToEmitFile(
				pass_manager,
				dest, 
				nullptr,
				llvm::CGFT_ObjectFile
			)) {
				return error::emit<5000>();
			}

			pass_manager.run(
				*llvm_context->get_module()
			);

			dest.flush();
		}

		// compile the .o file with clang
		// create the compiler 
		const llvm::IntrusiveRefCntPtr diagnostic_options = new clang::DiagnosticOptions;
		auto* diagnostic_client = new clang::TextDiagnosticPrinter(
			llvm::errs(), 
			&*diagnostic_options
		);

		const llvm::IntrusiveRefCntPtr diagnostic_id(
			new clang::DiagnosticIDs()
		);

		clang::DiagnosticsEngine diagnostics_engine(
			diagnostic_id,
			&*diagnostic_options, 
			diagnostic_client
		);

		clang::driver::Driver driver(
			"/usr/bin/clang++-12", 
			target_triple, 
			diagnostics_engine
		);

		// generate clang arguments
		const std::vector argument_vector{
			"-g",
			o_file.c_str(),
			"-o",
			exe_file.c_str()
		};

		// run the compiler 
		const llvm::ArrayRef arguments(
			argument_vector
		);

		const std::unique_ptr<clang::driver::Compilation> compilation(
			driver.BuildCompilation(arguments)
		);

		// check for compilation errors
		if (compilation) {
			if (compilation->containsError()) {
				error::emit<5001>();
			}

			llvm::SmallVector<std::pair<i32, const clang::driver::Command*>, 4> failing_commands;
			driver.ExecuteCompilation(
				*compilation,
				failing_commands
			);
		}

		// delete the .o file
		if (!detail::delete_file(o_file)) {
			return error::emit<1001>(o_file);
		}

		return {};
	}

	error_result compiler::verify_source_file(
		const filepath& path
	) {
		if (!std::filesystem::exists(path)) {
			return error::emit<1002>(path);
		}

		if (!detail::is_file(path)) {
			return error::emit<1003>(path);
		}

		if (detail::extract_extension_from_filepath(path) != ".ch") {
			return error::emit<1007>(path, ".ch");
		}

		return {};
	}

	error_result compiler::verify_folder(
		const filepath& folder_path
	) {
		if (!std::filesystem::exists(folder_path)) {
			return error::emit<1002>(folder_path);
		}

		if(!detail::is_directory(folder_path)) {
			return error::emit<1004>(folder_path);
		}

		return {};
	}
}
