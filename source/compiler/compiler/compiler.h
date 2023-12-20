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

namespace sigma {
	using namespace utility::types;

	namespace ir { 
		class module;
	}

	class compiler {
	public:
		static void compile(const filepath& path, ir::target target);
	private:
		compiler(const filepath& path, ir::target target);

		void compile() const;
		auto get_object_file_path(const std::string& name = "a") const -> filepath;

		static void verify_file(const filepath& path);
		static void emit_object_file(ir::module& module, const filepath& path);
	private:
		filepath m_path;
		ir::target m_target;
	};
} // namespace sigma
