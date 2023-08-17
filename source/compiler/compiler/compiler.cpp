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
#include "string_helper.h"
#include "timer.h"

namespace sigma {
	compiler::compiler() {}

	outcome::result<void> compiler::compile(
		const compiler_settings& settings
	) {
		m_settings = settings;

		timer compilation_timer;
		compilation_timer.start();
		
		console::out
			<< "sigma: "
			<< color::yellow
			<< "info: "
			<< color::white
			<< "constructing dependency tree...\n";
		
		dependency_tree tree(m_settings.root_source_file);
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
		const filepath& object_file
	) {
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
		const std::string executable_file_str = m_settings.outputs[file_extension::exe].string();

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
	) {
		filepath object_file;
		const auto o_it = m_settings.outputs.find(file_extension::o);
		if(o_it != m_settings.outputs.end()) {
			object_file = o_it->second;
		}
		else {
			object_file = "./temp.o";
		}

		filepath executable_file;

		// attempt to create the target machine
		OUTCOME_TRY(const auto target_machine, create_target_machine(context));

		// generate the .o file
		OUTCOME_TRY(create_object_file(object_file, target_machine, context));

		// only compile to an executable if we have a valid output filepath
		if(m_settings.outputs.contains(file_extension::exe)) {
			// compile the .o file with clang
			OUTCOME_TRY(compile_object_file(target_machine->getTargetTriple(), object_file));
		}

		if(!m_settings.outputs.contains(file_extension::o)) {
			// delete .o file if we don't want to emit it 
			OUTCOME_TRY(file::remove(object_file));
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

	outcome::result<compiler_settings> compiler_settings::parse_argument_list(
		argument_list& arguments
	) {
		compiler_settings settings;

		// files
		// source file
		settings.root_source_file = arguments.get<std::string>("source");

		// executable file
		const auto outputs = arguments.get<std::vector<std::string>>("output");

		for(const auto& output : outputs) {
			filepath path = output;
			if(path.extension() == ".exe") {
				settings.outputs[file_extension::exe] = path;
			}
			else if (path.extension() == ".o") {
				settings.outputs[file_extension::o] = path;
			}
			else if (path.extension() == ".llvm") {
				settings.outputs[file_extension::llvm] = path;
			}
			else {
				return outcome::failure(
					error::emit<error_code::unrecognized_output_extension>(path.extension())
				);
			}
		}

		// performance optimization levels
		if(arguments.get<bool>("optimize0")) {
			settings.opt_level = optimization_level::none;
		}

		if (arguments.get<bool>("optimize1")) {
			settings.opt_level = optimization_level::low;
		}

		if (arguments.get<bool>("optimize2")) {
			settings.opt_level = optimization_level::medium;
		}

		if (arguments.get<bool>("optimize3")) {
			settings.opt_level = optimization_level::high;
		}

		// size optimization levels
		if (arguments.get<bool>("size-optimize0")) {
			settings.size_opt_level = size_optimization_level::none;
		}

		if (arguments.get<bool>("size-optimize1")) {
			settings.size_opt_level = size_optimization_level::medium;
		}

		if (arguments.get<bool>("size-optimize2")) {
			settings.size_opt_level = size_optimization_level::high;
		}

		// vectorization
		if (arguments.get<bool>("fslp-vectorize")) {
			settings.vectorize = true;
		}

		return settings;
	}

	compiler_emit_types operator|(compiler_emit_types lhs, compiler_emit_types rhs) {
		return static_cast<compiler_emit_types>(
			static_cast<std::underlying_type_t<compiler_emit_types>>(lhs) |
			static_cast<std::underlying_type_t<compiler_emit_types>>(rhs));
	}

	compiler_emit_types operator&(compiler_emit_types lhs, compiler_emit_types rhs) {
		return static_cast<compiler_emit_types>(
			static_cast<std::underlying_type_t<compiler_emit_types>>(lhs) &
			static_cast<std::underlying_type_t<compiler_emit_types>>(rhs));
	}
}
