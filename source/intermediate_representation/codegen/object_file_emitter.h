#pragma once
#include <utility/filesystem/new/file_types/object_file.h>

namespace sigma::ir {
	using namespace utility::types;

	class module;
	class object_file_emitter {
	public:
		virtual utility::object_file emit(module& module) = 0;
		virtual ~object_file_emitter() = default;
	};
}
