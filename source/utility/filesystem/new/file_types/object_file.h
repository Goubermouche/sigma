#pragma once
#include "utility/containers/contiguous_container.h"
#include "utility/memory.h"

namespace utility {
	class object_file : public contiguous_container<byte> {};
}
