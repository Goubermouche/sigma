#pragma once
#include "diagnostics/error.h"
#include "codegen/visitor/codegen_visitor.h"

namespace channel {
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

	struct compiler_settings {
		// optimization level, higher optimization levels can result in higher performance
		optimization_level optimization_level = optimization_level::none;
		// size optimization levels, higher size optimization levels can result in smaller executable size
		size_optimization_level size_optimization_level = size_optimization_level::none;
		// vectorize loops and enable auto vectorization 
		bool vectorize = false;
	};

	/**
	 * \brief Compiler instance, used for compiling channel files into an executable.
	 */
	class compiler {
	public:
		/**
		 * \brief Constructs a new compiler instance using the specified \a settings.
		 * \param settings Settings to use when compiling 
		 */
		compiler(
			compiler_settings settings
		);

		/**
		 * \brief Compiles the given \a root_source_file_filepath using the underlying compiler settings and outputs an executable at the given \a target_executable_directory.
		 * \param root_source_file_filepath Path to the file to be compiled
		 * \param target_executable_directory Path to the target directory the generated executable should be store to
		 * \return Optional error message containing information about a potential error.
		 */
		error_result compile(
			const std::string& root_source_file_filepath,
			const std::string& target_executable_directory
		);
	private:
		std::expected<std::shared_ptr<llvm::Module>, error_message> generate_module(
			const std::string& source_filepath
		);

		error_result compile_module(
			const std::shared_ptr<llvm::Module>& module
		) const;

		static error_result verify_source_file(
			const std::string& filepath
		);

		static error_result verify_folder(
			const std::string& folder_filepath
		);
	private:
		compiler_settings m_settings;

		// compilation specific 
		std::shared_ptr<codegen_visitor> m_active_visitor;
		std::string m_root_source_file_filepath;
		std::string m_target_executable_directory;
	};
}