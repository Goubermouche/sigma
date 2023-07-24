#include "compiler.h"
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

#include "utility/string.h"

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
			<< "constructing dependency graph...\n";
		
		dependency_graph graph(m_root_source_path);
		OUTCOME_TRY(graph.construct());

		console::out
			<< "sigma: "
			<< color::yellow
			<< "info: "
			<< color::white
			<< "verifying dependency graph...\n";

		OUTCOME_TRY(graph.verify());

		console::out
			<< "sigma: "
			<< color::yellow
			<< "info: "
			<< color::white
			<< "parsing dependency graph...\n";

		OUTCOME_TRY(
			const std::shared_ptr<abstract_syntax_tree>& abstract_syntax_tree, 
			graph.parse()
		);

		// abstract_syntax_tree->print_nodes();

		console::out
			<< "sigma: "
			<< color::yellow
			<< "info: "
			<< color::white
			<< "generating code...\n";

		code_generator codegen;
		OUTCOME_TRY(const std::shared_ptr<code_generator_context>& context, codegen.generate(abstract_syntax_tree));

		// verify the executable directory
		OUTCOME_TRY(verify_folder(m_target_executable_directory));

		console::out
			<< "sigma: "
			<< color::yellow
			<< "info: "
			<< color::white
			<< "compiling "
			<< graph.size()
			<< ' '
			<< detail::format_ending(graph.size(), "file", "files")
			<< "...\n";
	
		// compile the module into an executable
		OUTCOME_TRY(compile_module(context));

		console::out
			<< "sigma: "
			<< color::yellow
			<< "info: "
			<< color::white
			<< "compilation finished ("
			<< precision(3)
			<< compilation_timer.elapsed<std::chrono::duration<f64>>()
			<< "s)\n";

		return outcome::success();
	}

	outcome::result<void> compiler::compile_module(
		const std::shared_ptr<code_generator_context>& llvm_context
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
				return outcome::failure(error::emit<5000>());
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
			return outcome::failure(error::emit<1001>(o_file));
		}

		return outcome::success();
	}

	outcome::result<void> compiler::verify_folder(
		const filepath& folder_path
	) {
		if (!exists(folder_path)) {
			return outcome::failure(error::emit<1002>(folder_path));
		}

		if(!detail::is_directory(folder_path)) {
			return outcome::failure(error::emit<1004>(folder_path));
		}

		return outcome::success();;
	}

	outcome::result<void> compiler::verify_main_context(
		const std::shared_ptr<code_generator_context>& context
	) {
		// check if we have a valid 'main' function
		if(const auto main_func = context->get_function_registry().get_function("main", context)) {
			if (main_func->get_return_type() != type(type::base::i32, 0)) {
				return outcome::failure(error::emit<4013>(main_func->get_return_type())); // return on failure
			}
		}
		else {
			return outcome::failure(error::emit<4012>()); // return on failure
		}

		// check for IR errors
		// if (llvm::verifyModule(*m_llvm_handler->get_module(), &llvm::outs())) {
		// 	console::out << color::white;
		// 	return error::emit<4016>();
		// }

		return outcome::success();
	}
}
