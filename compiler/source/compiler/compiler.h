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

	struct compiler_description {
		std::string root_source_file;
		std::string executable_location;
		optimization_level optimization_level = optimization_level::none;
		size_optimization_level size_optimization_level = size_optimization_level::none;
		bool vectorize = false;
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