#pragma once
#include "intermediate_representation/target/parameter_descriptor.h"

namespace ir {
	enum class arch {
		X64
	};

	enum class system {
		WINDOWS,
		LINUX,
		MAC_OS,
		ANDROID,
		WEB
	};

	enum class abi {
		WIN_64,
		SYSTEMV
	};

	class target {
	public:
		target() = default;
		target(arch arch, system system);

		[[nodiscard]] auto get_abi() const->abi;
		[[nodiscard]] auto get_arch() const->arch;
		[[nodiscard]] auto get_system() const->system;

		[[nodiscard]] auto get_parameter_descriptor() const -> parameter_descriptor;
	private:
		abi m_abi;
		arch m_arch;
		system m_system;
	};
}
