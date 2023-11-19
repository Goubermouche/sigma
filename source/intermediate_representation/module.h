#pragma once
#include "intermediate_representation/node_hierarchy/function.h"
#include "intermediate_representation/codegen/targets/target.h"

#include <utility/containers/byte_buffer.h>
#include <utility/containers/string.h>
#include <utility/diagnostics/error.h>

// The entire IR system is based off of an implementation in Cuik's Tilde backend
// (https://github.com/RealNeGate/Cuik/tree/master/tb)

// Root code generation file, contains the main module structure. Each module
// contains a list of functions, and every function contains an allocator through
// which it allocates and stores nodes, which represent the operation tree of the
// given function. 

namespace ir {
	struct codegen_result {
		utility::byte_buffer bytecode;
		utility::string assembly;
	};

	// general memory model:
	// every function keeps its own block of memory, which is currently allocated
	// in a linear, and unaligned fashion.
	// todo: implement a more performant, aligned memory allocator 

	/**
	 * \brief Basic builder which serves as a facade around the process of
	 * constructing proper IR.
	 */
	class module {
	public:
		~module();

		auto compile(arch arch, system system) const -> codegen_result;

		auto create_function(
			const std::string& identifier,
			const std::vector<data_type>& parameters, 
			const std::vector<data_type>& returns
		) -> handle<function>;
	private:
		std::vector<handle<function>> m_functions;
	};
}
