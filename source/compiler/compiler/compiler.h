// Root compiler file. Contains the core implementation of the compiler and the related
// sub-processes. The main invocation point is located in the main.cpp file. The required
// steps which need to be taken to create a valid compilation together with their specific
// order can be seen below:
//
// AST generation                           Lowering (IR)      Lowering (Machine bytecode)
// +-------+  +--------+  +--------------+  +---------------+  +----------------+  +------------+
// | lexer +->| parser +->+ type_checker +->+ IR_translator +->| code_generator +->| executable |
// +-------+  +--------+  +--------------+  +---------------+  +----------------+  +------------+

#pragma once
#include <intermediate_representation/target/target.h>
#include <utility/diagnostics.h>
#include <utility/macros.h>

namespace sigma {
	using namespace utility::types;

	namespace ir { 
		class module;
	} // namespace sigma::ir

	struct compiler_description {
		filepath path;
		ir::target target;
	};

	class compiler {
	public:
		static auto compile(const compiler_description& description) -> utility::result<void>;
	private:
		compiler(const compiler_description& description);

		auto compile() const -> utility::result<void>;
		auto get_object_file_path(const std::string& name = "a") const -> filepath;

		static auto verify_file(const filepath& path) -> utility::result<void>;
		static void emit_object_file(ir::module& module, const filepath& path);
	private:
		compiler_description m_description;
	};
} // namespace sigma
