#include "compiler.h"
#include "../codegen/visitor/codegen_visitor.h"

#include "llvm/Support/VirtualFileSystem.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Compilation.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"

#define EXE_PATH "main.exe"

namespace channel {
	void compiler::compile(const std::string& source_file) {
		std::cout << "compiling file '" << source_file << "'\n";

		timer timer;
		timer.start();

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

		const std::shared_ptr<llvm::Module> module = visitor.get_module();

        const auto target_triple = llvm::sys::getDefaultTargetTriple();
		llvm::InitializeAllTargetInfos();
		llvm::InitializeAllTargets();
		llvm::InitializeAllTargetMCs();
		llvm::InitializeAllAsmParsers();
		llvm::InitializeAllAsmPrinters();

		std::string Error;
		const auto target = llvm::TargetRegistry::lookupTarget(target_triple, Error);
		const auto cpu = "generic";
		const auto features = "";

		const llvm::TargetOptions target_options;
		constexpr auto relocation_model = llvm::Optional<llvm::Reloc::Model>();
		const auto target_machine = target->createTargetMachine(target_triple, cpu, features, target_options, relocation_model);

		module->setDataLayout(target_machine->createDataLayout());
		module->setTargetTriple(target_triple);

		constexpr auto file_name = "output.o";

		std::cout << "creating object files\n";

		{
			std::error_code error_code;
			llvm::raw_fd_ostream dest(file_name, error_code, llvm::sys::fs::OF_None);

			llvm::legacy::PassManager pass;
			constexpr auto file_type = llvm::CGFT_ObjectFile;

			if (target_machine->addPassesToEmitFile(pass, dest, nullptr, file_type)) {
				llvm::errs() << "the target machine can't emit a file of this type\n";
				return;
			}

			pass.run(*module);
			dest.flush();
		}

		std::cout << "object files created\n";

		const llvm::IntrusiveRefCntPtr diagnostic_options = new clang::DiagnosticOptions;
		auto* diagnostic_client = new clang::TextDiagnosticPrinter(llvm::errs(), &*diagnostic_options);
		const llvm::IntrusiveRefCntPtr diagnostic_id(new clang::DiagnosticIDs());
		clang::DiagnosticsEngine diagnostics_engine(diagnostic_id, &*diagnostic_options, diagnostic_client);
		clang::driver::Driver driver("/usr/bin/clang++-12", target_triple, diagnostics_engine);

		const std::vector<const char*> argument_vector {
			"-g",
			"output.o",
			"-o",
			"main.exe"/*,
			"-v"*/
		};

		const llvm::ArrayRef<const char*> arguments(argument_vector);
		const std::unique_ptr<clang::driver::Compilation> compilation(driver.BuildCompilation(arguments));

		if (compilation && !compilation->containsError()) {
			llvm::SmallVector<std::pair<int, const clang::driver::Command*>, 4> failing_commands;
			driver.ExecuteCompilation(*compilation, failing_commands);
		}

		remove(file_name);

		std::cout << "successfully compiled in " << timer.elapsed() << "ms\n";
	}
}