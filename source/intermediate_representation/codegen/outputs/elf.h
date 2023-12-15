#pragma once
#include "intermediate_representation/codegen/object_file_emitter.h"

namespace sigma::ir {
	class elf_file_emitter : public object_file_emitter {
	public:
		utility::object_file emit(module& module) override;
	private:
	};
} // namespace sigma::ir

