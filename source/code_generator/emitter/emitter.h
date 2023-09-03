#pragma once
#include <intermediate_representation/intermediate_representation.h>

namespace code_generator {
	class emitter {
	public:
		virtual ~emitter() = default;

		virtual std::vector<utility::byte> emit(
			const ir::builder& builder
		) = 0;
	};
}
