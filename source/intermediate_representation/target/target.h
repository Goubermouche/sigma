#pragma once
#include "intermediate_representation/target/parameter_descriptor.h"

namespace sigma::ir {
	enum class arch : u8 {
		X64
	};

	enum class system : u8 {
		WINDOWS,
		LINUX
	};

	enum class abi : u8 {
		WIN_64,
		SYSTEMV
	};

	/**
	 * \brief Represents the target execution platform (architecture x system x ABI).
	 */
	class target {
	public:
		target() = default;
		target(arch arch, system system);

		[[nodiscard]] auto get_abi() const -> abi;
		[[nodiscard]] auto get_arch() const -> arch;
		[[nodiscard]] auto get_system() const -> system;

		[[nodiscard]] auto get_parameter_descriptor() const -> parameter_descriptor;
	private:
		abi m_abi;
		arch m_arch;
		system m_system;
	};
}
