#pragma once
#include "diagnostics/error.h"
#include "codegen/visitor/codegen_visitor.h"


namespace channel {
	struct compiler_description {
		std::string root_source_file;
		std::string executable_location;
		u8 optimization_level; // performance optimization level (0 - 3)
		u8 size_optimization_level; // size optimization level (0 - 2)
	};

	class compiler {
	public:
		compiler(
			compiler_description description
		);

		std::optional<error_message> compile();
	private:
		std::expected<std::shared_ptr<llvm::Module>, error_message> generate_module(
			const std::string& source_filepath
		);

		std::optional<error_message> compile_module(
			const std::shared_ptr<llvm::Module>& module
		) const;

		static std::optional<error_message> verify_source_file(
			const std::string& filepath
		);

		static std::optional<error_message> verify_folder(
			const std::string& folder_filepath
		);
	private:
		compiler_description m_description;
		std::shared_ptr<codegen_visitor> m_active_visitor;
	};
}