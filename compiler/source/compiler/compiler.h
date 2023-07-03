#pragma once

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "code_generator/code_generator.h"

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
		 * \brief Sets the new lexer that the compiler will use for tokenizing the source code.
		 * \tparam lexer Lexer to use
		 */
		template <typename lexer>
		void set_lexer();

		/**
		 * \brief Sets the new parser that the compiler will use for generating the AST. 
		 * \tparam parser Parser to use
		 */
		template <typename parser>
		void set_parser();

		/**
		 * \brief Sets the new code generator that the compiler will use for generating LLVM IR.
		 * \tparam code_generator Code generator to use 
		 */
		template <typename code_generator>
		void set_code_generator();

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
		std::expected<
			std::shared_ptr<llvm_context>,
			error_msg
		> generate_module(
			const std::string& source_filepath
		) const;

		error_result compile_module(
			const std::shared_ptr<llvm_context>& llvm_context
		) const;

		static error_result verify_source_file(
			const std::string& filepath
		);

		static error_result verify_folder(
			const std::string& folder_filepath
		);
	private:
		compiler_settings m_settings;

		// lexer to use for tokenization of the source file
		std::function<std::shared_ptr<lexer>()> m_lexer_generator;
		// parser to use for generating the AST
		std::function<std::shared_ptr<parser>()> m_parser_generator;
		// code generator used for generating LLVM IR
		std::function<std::shared_ptr<code_generator>()> m_code_generator_generator;

		// compilation specific 
		std::string m_root_source_file_filepath;
		std::string m_target_executable_directory;
	};

	template<typename lexer>
	void compiler::set_lexer() {
		m_lexer_generator = [] {
			return std::make_shared<lexer>();
		};
	}

	template<typename parser>
	void compiler::set_parser() {
		m_parser_generator = [] {
			return std::make_shared<parser>();
		};
	}

	template<typename code_generator>
	void compiler::set_code_generator() {
		m_code_generator_generator = [] {
			return std::make_shared<code_generator>();
		};
	}
}