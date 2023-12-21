#pragma once

namespace sigma::ir {
	struct codegen_context;

	class allocator_base {
	public:
		virtual void allocate(codegen_context& context) = 0;
		virtual ~allocator_base() = default;
	};
} // namespace sigma::ir
