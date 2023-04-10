#include "compiler.h"
#include "../codegen/visitor/codegen_visitor.h"

// llvm
#include "llvm/Support/VirtualFileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Host.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

// clang
#include "clang/Driver/Driver.h"
#include "clang/Driver/Compilation.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"

namespace channel {
	void compiler::compile(const std::string& source_file) {
		std::cout << "compiling file '" << source_file << "'\n";

		timer timer;
		timer.start();

		// generate LLVM IR
		parser parser(source_file);
		codegen_visitor visitor;

		if (!visitor.generate(parser)) {
			// codegen failure
			return;
		}

		if(!visitor.verify_intermediate_representation()) {
			// verification failure
			return;
		}

		// compile the file into an exe
		// get the generated module
		const std::shared_ptr<llvm::Module> module = visitor.get_module();
        const std::string target_triple = llvm::sys::getDefaultTargetTriple();

		// initialize LLVM targets
		llvm::InitializeAllTargetInfos();
		llvm::InitializeAllTargets();
		llvm::InitializeAllTargetMCs();
		llvm::InitializeAllAsmParsers();
		llvm::InitializeAllAsmPrinters();

		// create the target machine 
		std::string error;
		const auto target = llvm::TargetRegistry::lookupTarget(target_triple, error);
		constexpr auto cpu = "generic";
		constexpr auto features = "";

		const llvm::TargetOptions target_options;
		constexpr auto relocation_model = llvm::Optional<llvm::Reloc::Model>();
		const auto target_machine = target->createTargetMachine(target_triple, cpu, features, target_options, relocation_model);

		module->setDataLayout(target_machine->createDataLayout());
		module->setTargetTriple(target_triple);

		const std::string o_file = "test/out.o";
		const std::string exe_file = "test/out.exe";

		// generate the .o file
		{
			std::error_code error_code;
			llvm::raw_fd_ostream dest(o_file, error_code, llvm::sys::fs::OF_None);
			llvm::legacy::PassManager pass_manager;
			llvm::PassManagerBuilder builder;

			// add optimization passes
			builder.OptLevel = 3; // set the optimization level (0-3)
			builder.SizeLevel = 0; // set the size optimization level (0-2)
			builder.Inliner = llvm::createFunctionInliningPass(builder.OptLevel, builder.SizeLevel, false);
			builder.LoopVectorize = true;
			builder.SLPVectorize = true;
			builder.populateModulePassManager(pass_manager);

			if (target_machine->addPassesToEmitFile(pass_manager, dest, nullptr, llvm::CGFT_ObjectFile)) {
				llvm::errs() << "the target machine can't emit a file of this type\n";
				return;
			}

			pass_manager.run(*module);
			dest.flush();
		}

		// compile the .o file with clang
		// create the compiler 
		const llvm::IntrusiveRefCntPtr diagnostic_options = new clang::DiagnosticOptions;
		auto* diagnostic_client = new clang::TextDiagnosticPrinter(llvm::errs(), &*diagnostic_options);
		const llvm::IntrusiveRefCntPtr diagnostic_id(new clang::DiagnosticIDs());
		clang::DiagnosticsEngine diagnostics_engine(diagnostic_id, &*diagnostic_options, diagnostic_client);
		clang::driver::Driver driver("/usr/bin/clang++-12", target_triple, diagnostics_engine);

		// generate clang arguments
		const std::vector argument_vector {
			"-g",
			o_file.c_str(),
			"-o",
			exe_file.c_str()
		};

		// run the compiler 
		const llvm::ArrayRef arguments(argument_vector);
		const std::unique_ptr<clang::driver::Compilation> compilation(driver.BuildCompilation(arguments));

		// check for compilation errors
		if (compilation) {
			if(!compilation->containsError()) {
				llvm::SmallVector<std::pair<int, const clang::driver::Command*>, 4> failing_commands;
				driver.ExecuteCompilation(*compilation, failing_commands);
			}
			else {
				return;
			}
		}

		// delete the .o file
		if(!detail::delete_file(o_file)) {
			compilation_logger::emit_delete_file_failed_error(o_file);
			return;
		}

		std::cout << "successfully compiled in " << timer.elapsed() << "ms\n";
	}
}