#pragma once
#include "code_generator/code_generator_base.h"

namespace code {
	class x64_code_generator : public code_generator_base {
	public:
		x64_code_generator(
			s_ptr<register_allocator_base> register_allocator
		);

		void compile_function(handle<ir::function> function) override;
	private:

	};
}
