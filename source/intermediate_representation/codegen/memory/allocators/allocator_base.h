#pragma once

namespace ir {
	struct codegen_context;

	class allocator_base {
	public:
		virtual void allocate(codegen_context& context) = 0;
		virtual ~allocator_base() = default;
	};
}