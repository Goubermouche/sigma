#pragma once

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "code_generator/code_generator.h"
#include "dependency_graph.h"

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

	struct compiler_settings {
		// optimization level, higher optimization levels can result in higher performance
		optimization_level optimization_level = optimization_level::none;
		// size optimization levels, higher size optimization levels can result in smaller executable size
		size_optimization_level size_optimization_level = size_optimization_level::none;
		// vectorize loops and enable auto vectorization 
		bool vectorize = false;
		// thread limit for any multithreaded operation
		// note: using only one thread doesn't currently work
		u32 thread_limit = 8;
	};

	/**
	 * \brief Compiler instance, used for compiling sigma files into an executable.
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
		 * \brief Sets the new code generator that the compiler will use for generating LLVM IR.
		 * \tparam code_generator Code generator to use 
		 */
		template <typename code_generator>
		void set_code_generator();

		/**
		 * \brief Compiles the given \a root_source_path using the underlying compiler settings and outputs an executable at the given \a target_executable_directory.
		 * \param root_source_path Path to the file to be compiled
		 * \param target_executable_directory Path to the target directory the generated executable should be store to
		 * \return Optional error message containing information about a potential error.
		 */
		outcome::result<void> compile(
			const filepath& root_source_path,
			const filepath& target_executable_directory
		);
	private:
		outcome::result<std::shared_ptr<llvm_context>> generate_module(
			const filepath& source_path
		);

		outcome::result<void> compile_module(
			const std::shared_ptr<llvm_context>& llvm_context
		) const;

		static outcome::result<void> verify_source_file(
			const filepath& path
		);

		static outcome::result<void> verify_folder(
			const filepath& folder_path
		);
	private:
		compiler_settings m_settings;

		lexer m_lexer;
		parser m_parser;
		std::function<std::shared_ptr<code_generator>()> m_code_generator_generator;

		// compilation specific 
		filepath m_root_source_path;
		filepath m_target_executable_directory;
	};

	template<typename code_generator>
	void compiler::set_code_generator() {
		m_code_generator_generator = [] {
			return std::make_shared<code_generator>();
		};
	}
}