#pragma once

#include "code_generator/implementation/code_generator.h"
#include "program_aruments/program_options.h"
#include "dependency_tree.h"

#include <llvm/Target/TargetMachine.h>

namespace sigma {
	enum class optimization_level {
		none = 0,
		low = 1,
		medium = 2,
		high = 3
	};

	enum class size_optimization_level {
		none = 0,
		medium = 1,
		high = 2
	};

	enum class compiler_emit_types {
		llvm_ir    = 1 << 1,
		object     = 1 << 2
	};

	enum class file_extension {
		exe,
		llvm,
		o
	};

	// operator overloads for bitwise operators
	compiler_emit_types operator|(compiler_emit_types lhs, compiler_emit_types rhs);
	compiler_emit_types operator&(compiler_emit_types lhs, compiler_emit_types rhs);

	struct compiler_settings {
		// file paths
		filepath root_source_file;
		std::unordered_map<file_extension, filepath> outputs;

		// optimization level, higher optimization levels can result in higher performance
		optimization_level optimization_level = optimization_level::none;
		// size optimization levels, higher size optimization levels can result in smaller executable size
		size_optimization_level size_optimization_level = size_optimization_level::none;
		// vectorize loops and enable auto vectorization 
		bool vectorize = false;

		static outcome::result<compiler_settings> parse_argument_list(
			argument_list& arguments
		);
	};

	/**
	 * \brief Compiler instance, used for compiling sigma files into an executable.
	 */
	class compiler {
	public:
		/**
		 * \brief Constructs a new compiler instance using the specified \a settings.
		 */
		compiler();

		/**
		 * \brief Compiles the given \a root_source_path using the underlying compiler settings and outputs an executable at the given \a target_executable_directory.
		 * \return Optional error message containing information about a potential error.
		 */
		outcome::result<void> compile(
			const compiler_settings& settings
		);
	private:
		outcome::result<void> create_object_file(
			const filepath& path,
			llvm::TargetMachine* machine,
			const std::shared_ptr<code_generator_context>& context
		) const;

		outcome::result<void> compile_object_file(
			const llvm::Triple& target_triple,
			const filepath& object_file
		);

		outcome::result<llvm::TargetMachine*> create_target_machine(
			const std::shared_ptr<code_generator_context>& context
		) const;

		outcome::result<void> compile_module(
			const std::shared_ptr<code_generator_context>& llvm_context
		);

		static outcome::result<void> verify_main_context(
			const std::shared_ptr<code_generator_context>& context
		);
	private:
		compiler_settings m_settings;
	};
}