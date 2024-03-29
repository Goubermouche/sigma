// Root compiler file. Contains the core implementation of the compiler and the related
// sub-processes. The main invocation point is located in the main.cpp file. The required
// steps which need to be taken to create a valid compilation together with their specific
// order can be seen below:
//                        +---------------------------------+
//                        | Type system                     |
//                        +---------------------------------+
// AST generation                           Lowering (IR)      Lowering (Machine bytecode)
// +-------+  +--------+  +--------------+  +---------------+  +----------------+  +------------+
// | lexer +->| parser +->+ type_checker +->+ IR_translator +->| code_generator +->| executable |
// +-------+  +--------+  +--------------+  +---------------+  +----------------+  +------------+

#pragma once
#include <intermediate_representation/target/target.h>
#include <utility/filesystem/filesystem.h>
#include <parametric/parametric.h>
#include <utility/diagnostics.h>

namespace sigma {
	using namespace utility::types;

	namespace ir { 
		class module;
	} // namespace sigma::ir

	enum class emit_target : u8 {
		NONE,
		OBJECT,
		EXECUTABLE
	};

	struct compiler_description {
		filepath source_path;
		filepath emit_path;
		ir::target target;
	};

	class compiler {
	public:
		static auto compile(const compiler_description& description) -> utility::result<void>;
	private:
		compiler(const compiler_description& description);

		auto compile() -> utility::result<void>;
		auto get_object_file_path(const std::string& name = "a") const -> filepath;

		static auto verify_file(const filepath& path) -> utility::result<void>;
		static auto emit_object_file(ir::module& module, const filepath& path) -> utility::result<void>;

		auto get_emit_target_from_path(const filepath& path) const -> utility::result<emit_target>;
	private:
		compiler_description m_description;
		emit_target m_emit_target = emit_target::NONE;
	};
} // namespace sigma

template<>
struct parametric::options_parser<sigma::filepath> {
	static auto parse(const std::string& value) -> sigma::filepath {
		return value;
	}
};

template<>
struct parametric::options_parser<sigma::ir::arch> {
	static auto parse(const std::string& value) -> sigma::ir::arch {
		if (value == "x64") {
			return sigma::ir::arch::X64;
		}

		throw std::invalid_argument("invalid argument");
	}
};

template<>
struct parametric::options_parser<sigma::ir::system> {
	static auto parse(const std::string& value) -> sigma::ir::system {
		if (value == "windows") {
			return sigma::ir::system::WINDOWS;
		}

		if (value == "linux") {
			return sigma::ir::system::LINUX;
		}

		throw std::invalid_argument("invalid argument");
	}
};

template<>
struct parametric::options_parser<sigma::emit_target> {
	static auto parse(const std::string& value) -> sigma::emit_target {
		if (value == "none") {
			return sigma::emit_target::NONE;
		}

		if (value == "object") {
			return sigma::emit_target::OBJECT;
		}

		if (value == "executable") {
			return sigma::emit_target::EXECUTABLE;
		}

		throw std::invalid_argument("invalid argument");
	}
};
