#include "compiler.h"

// llvm
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Host.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

// clang
#include <clang/Driver/Driver.h>
#include <clang/Driver/Compilation.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <llvm/IR/Verifier.h>

// utility
#include "utility/string.h"
#include "utility/timer.h"

namespace sigma {
	compiler::compiler(
		compiler_settings settings
	) : m_settings(settings) {}

	outcome::result<void> compiler::compile(
		const filepath& root_source_path,
		const filepath& target_executable_directory
	) {
		timer compilation_timer;
		compilation_timer.start();
		
		m_root_source_path = root_source_path;
		m_target_executable_directory = target_executable_directory;

		console::out
			<< "sigma: "
			<< color::yellow
			<< "info: "
			<< color::white
			<< "constructing dependency tree...\n";
		
		dependency_tree tree(m_root_source_path);
		OUTCOME_TRY(tree.construct());

		console::out
			<< "sigma: "
			<< color::yellow
			<< "info: "
			<< color::white
			<< "verifying dependency tree...\n";

		OUTCOME_TRY(tree.verify());

		console::out
			<< "sigma: "
			<< color::yellow
			<< "info: "
			<< color::white
			<< "parsing dependency tree...\n";

		OUTCOME_TRY(
			const std::shared_ptr<abstract_syntax_tree>& abstract_syntax_tree, 
			tree.parse()
		);

		// abstract_syntax_tree->print_nodes();

		console::out
			<< "sigma: "
			<< color::yellow
			<< "info: "
			<< color::white
			<< "generating code...\n";

		code_generator codegen(abstract_syntax_tree);
		OUTCOME_TRY(
			const std::shared_ptr<code_generator_context>& context, 
			codegen.generate()
		);

		// verify the executable directory
		OUTCOME_TRY(verify_folder(m_target_executable_directory));

		console::out
			<< "sigma: "
			<< color::yellow
			<< "info: "
			<< color::white
			<< "compiling "
			<< tree.size()
			<< ' '
			<< detail::format_ending(tree.size(), "file", "files")
			<< "...\n";

		// context->get_module()->print(llvm::outs(), nullptr);

		// verify the context
		OUTCOME_TRY(verify_main_context(context));
	
		// compile the module into an executable
		OUTCOME_TRY(compile_module(context));

		console::out
			<< "sigma: "
			<< color::yellow
			<< "info: "
			<< color::white
			<< "compilation finished ("
			<< console::precision(3)
			<< compilation_timer.elapsed<std::chrono::duration<f64>>()
			<< "s)\n";

		return outcome::success();
	}

	outcome::result<void> compiler::create_object_file(
		const filepath& path,
		llvm::TargetMachine* machine,
		const std::shared_ptr<code_generator_context>& context
	) const {
		OUTCOME_TRY(const auto & object_file, raw_file::create(path));

		llvm::legacy::PassManager pass_manager;
		llvm::PassManagerBuilder builder;

		// add optimization passes
		builder.OptLevel = static_cast<u32>(m_settings.opt_level);
		builder.SizeLevel = static_cast<u32>(m_settings.size_opt_level);
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

		if (machine->addPassesToEmitFile(
			pass_manager,
			*object_file->get_stream(),
			nullptr,
			llvm::CGFT_ObjectFile
		)) {
			return outcome::failure(
				error::emit<error_code::target_machine_cannot_emit_file>()
			);
		}

		pass_manager.run(
			*context->get_module()
		);

		object_file->write();
		return outcome::success();
	}

	outcome::result<void> compiler::compile_object_file(
		const llvm::Triple& target_triple,
		const filepath& object_file,
		const filepath& executable_file
	) const {
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
			target_triple.str(),
			diagnostics_engine
		);

		// note: storing a string variant of the path appears to be necessary for some reason
		const std::string object_file_str = object_file.string();
		const std::string executable_file_str = executable_file.string();

		// generate clang arguments
		const std::vector argument_vector = {
			"-g",
			object_file_str.c_str(),
			"-o",
			executable_file_str.c_str()
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
				error::emit<error_code::clang_compilation_contains_errors>();
			}

			llvm::SmallVector<std::pair<i32, const clang::driver::Command*>, 4> failing_commands;
			driver.ExecuteCompilation(
				*compilation,
				failing_commands
			);
		}

		return outcome::success();
	}

	outcome::result<llvm::TargetMachine*> compiler::create_target_machine(
		const std::shared_ptr<code_generator_context>& context
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

		if(!error.empty()) {
			return outcome::failure(
				error::emit<error_code::cannot_lookup_target>(target_triple, error)
			);
		}

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

		context->get_module()->setDataLayout(
			target_machine->createDataLayout()
		);

		context->get_module()->setTargetTriple(
			target_triple
		);

		return target_machine;
	}

	outcome::result<void> compiler::compile_module(
		const std::shared_ptr<code_generator_context>& context
	) const {
		const filepath executable_file = m_target_executable_directory / "a.exe";
		const filepath object_file = m_target_executable_directory / "a.o";

		// attempt to create the target machine
		OUTCOME_TRY(const auto target_machine, create_target_machine(context));

		// generate the .o file
		OUTCOME_TRY(create_object_file(object_file, target_machine, context));

		// compile the .o file with clang
		OUTCOME_TRY(compile_object_file(target_machine->getTargetTriple(), object_file, executable_file));

		// delete the .o file
		return file::remove(object_file);
	}

	outcome::result<void> compiler::verify_folder(
		const filepath& folder_path
	) {
		if (!exists(folder_path)) {
			return outcome::failure(
				error::emit<error_code::file_does_not_exist>(folder_path)
			);
		}

		if(!is_directory(folder_path)) {
			return outcome::failure(
				error::emit<error_code::directory_expected_but_got_file>(folder_path)
			);
		}

		return outcome::success();
	}

	outcome::result<void> compiler::verify_main_context(
		const std::shared_ptr<code_generator_context>& context
	) {
		// check if we have a valid 'main' function
		if(const auto main_func = context->get_function_registry().get_function("main", context)) {
			if (main_func->get_return_type() != type(type::base::i32, 0)) {
				return outcome::failure(
					error::emit<error_code::cannot_find_main_with_correct_return_type>(main_func->get_return_type())
				); // return on failure
			}
		}
		else {
			return outcome::failure(
				error::emit<error_code::cannot_find_main>()
			); // return on failure
		}

		if (verifyModule(*context->get_module(), &llvm::errs())) {
			console::out
				<< "sigma: "
				<< color::yellow
				<< "info: "
				<< color::red
				<< "module verification failed\n";
			return outcome::success();
		}
		
		console::out
			<< "sigma: "
			<< color::yellow
			<< "info: "
			<< color::green
			<< "module verified successfully\n"
			<< color::white;

		return outcome::success();
	}
}
