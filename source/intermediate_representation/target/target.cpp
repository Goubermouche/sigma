#include "target.h"
#include "intermediate_representation/target/system/systemv/systemv.h"
#include "intermediate_representation/target/system/win/win.h"

namespace sigma::ir {
	target::target(arch arch, system system)
		: m_arch(arch), m_system(system) {
		m_abi = system == system::WINDOWS ? abi::WIN_64 : abi::SYSTEMV;
	}

	auto target::get_abi() const -> abi {
		return m_abi;
	}

	auto target::get_arch() const -> arch {
		return m_arch;
	}

	auto target::get_system() const -> system {
		return m_system;
	}

	auto target::get_parameter_descriptor() const -> parameter_descriptor {
		switch(m_abi) {
			case abi::SYSTEMV: return systemv::parameter_descriptor;
			case abi::WIN_64:  return win::parameter_descriptor;
		}

		NOT_IMPLEMENTED();
		return {};
	}
}
